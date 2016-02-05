/*

    COPYRIGHT AND PERMISSION NOTICE
    Copyright (c) 2015-2016 miVersa Inc
    All rights reserved.
    Permission to use, copy, modify, and distribute this software for any
    purpose with or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
    OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
    USE OR OTHER DEALINGS IN THE SOFTWARE.

    Except as contained in this notice, the name of a copyright holder shall
    not be used in advertising or otherwise to promote the sale, use or other
    dealings in this Software without prior written authorization of the
    copyright holder.

 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "ov_jsmn.h"
#include "ov_api.h"
#include "ov_curl.h"

#define OV_GET_MESSAGES_BY_ID_DEFAULT_COUNT 10

/**
 *  trim escape sequence from text
 */

void ov_trim(char *buf)
{
	char *out = buf, *in = buf;
	while (*in) {
		if (*in == '\\') {
			int i, oval = 0, special = 1;
			switch (*++in) {
			case 'a':	*out++ = '\a';	break;
			case 'b':	*out++ = '\b';	break;
			case 'f':	*out++ = '\f';	break;
			case 'n':	*out++ = '\n';	break;
			case 'r':	*out++ = '\r';	break;
			case 't':	*out++ = '\t';	break;
			case 'v':	*out++ = '\v';	break;
			default:
				/* check for octal */
				special = 0;
				for (i = 3; i && (*in >= '0') && (*in <= '7'); i--, in++) {
					oval = (oval << 3) | (*in - '0');
				}
				*out++ = i? *in++ : oval;
				break;
			}
			if (special) {
                in++;
			}
		}
		else {
			*out++ = *in++;
		}
	}
	*out = 0;
}

void ov_msg_cleanup(ov_msg_handle_t *ov_msg_handle)
{
    return ov_curl_msg_cleanup(ov_msg_handle);
}

int ov_group_name_to_id(ov_handle_t *ov_handle, char *group_name)
{
    ov_get_group_details_t ggd = {};
    ov_msg_handle_t *msg_handle = ov_get_group_details(ov_handle, group_name, &ggd);
    if (msg_handle) {
        /* success */
        ov_msg_cleanup(msg_handle);
    }
    else {
        printf("get group details failed\n");
        return -1;
    }
    return ggd.id;
}

/**
 *
 * Get [shared] group list
 */

static void ov_group_list_json_parse(void *ov_msg_handle, jsmntok_t *tokens,
                                     char *data, void *user_data)
{
    jsmntok_t *t;

    if ((t = jsmn_tok(tokens, data, "{result"))) {
        int count = t->size;
        ov_group_list_param_t *glt = (ov_group_list_param_t*) user_data;
        ov_group_list_status_t *gs = glt->entries = calloc(count,
                                     sizeof(ov_group_list_status_t));
        glt->entry_count = count;
        t = jsmn_tok(t, data, "[");
        while (t && count--) {
            gs->name         = jsmn_get_string(t, data, "Group_Name");
            gs->date_created = jsmn_get_string(t, data, "Date_Created");
            gs->id           = jsmn_get_int(t, data, "Group_ID");
            gs->chat_count   = jsmn_get_int(t, data, "Chat_Count");
            gs->scheduled_count = jsmn_get_int(t, data, "Scheduled_Count");
            gs->avatar       = jsmn_get_int(t, data, "Avatar");
            gs++;
            t = jsmn_skip(t);
        }
    }
}

static void ov_group_list_json_cleanup(void *data)
{
    ov_group_list_param_t *glt = data;

    if (glt->entries) {
        ov_group_list_status_t *gs = glt->entries;
        while (glt->entry_count--) {
            free(gs->name);
            free(gs->date_created);
            gs++;
        }
        free(glt->entries);
        glt->entries = NULL;
    }
    glt->entry_count = 0;
}

static ov_msg_handle_t *ov_get_group_list_common(ov_handle_t *handle,
        ov_group_list_param_t *glp, char *command)
{
    ov_msg_handle_t *msg_handle = ov_curl_msg_init(handle, command,
                                  ov_group_list_json_cleanup, glp);

    if (msg_handle) {
        ov_curl_set_int(msg_handle, "doBadges", glp->do_badges);
        if (ov_curl_json(msg_handle, ov_group_list_json_parse, glp)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}

ov_msg_handle_t *ov_get_group_list(ov_handle_t *handle,
                                   ov_group_list_param_t *glp)
{
    return ov_get_group_list_common(handle, glp, "getGroupList");
}

ov_msg_handle_t *ov_get_shared_group_list(ov_handle_t *handle,
        ov_group_list_param_t *glp)
{
    return ov_get_group_list_common(handle, glp, "getOtherGroups");
}

/**
 *  Get member list
 */

static void ov_member_list_json_parse(void *ov_msg_handle, jsmntok_t *tokens,
                                      char *data, void *user_data)
{
    jsmntok_t *t;

    if ((t = jsmn_tok(tokens, data, "{result"))) {
        int count = t->size;
        ov_member_list_t *glp = (ov_member_list_t*) user_data;
        ov_member_status_t *gs = glp->entries = calloc(count,
                                                sizeof(ov_member_status_t));
        glp->entry_count = count;
        t = jsmn_tok(t, data, "[");
        while (t && count--) {
            gs->name         = jsmn_get_string(t, data, "Member_Name");
            gs->id           = jsmn_get_int(t, data, "Member_Name_ID");
            gs->phone_mobile = jsmn_get_string(t, data, "Phone_Mobile");
            gs->email        = jsmn_get_string(t, data, "Email");
            gs->first_name   = jsmn_get_string(t, data, "First_Name");
            gs->last_name    = jsmn_get_string(t, data, "Last_Name");
            gs->contact_means = jsmn_get_int(t, data, "Contact_Means");
            gs++;
            t = jsmn_skip(t);
        }
    }
}

void ov_member_list_json_cleanup(void *data)
{
    ov_member_list_t *mlp = data;
    if (mlp->entries) {
        ov_member_status_t *me = mlp->entries;
        while (mlp->entry_count--) {
            free(me->name);
            free(me->phone_mobile);
            free(me->email);
            free(me->first_name);
            free(me->last_name);
            me++;
        }
        free(mlp->entries);
        mlp->entries = NULL;
    }
    mlp->entry_count = 0;
}

ov_msg_handle_t *ov_get_member_list(ov_handle_t *handle, ov_member_list_t *mlp)
{
    ov_msg_handle_t *msg_handle = ov_curl_msg_init(handle, "getMembers",
                                  ov_member_list_json_cleanup, mlp);

    if (msg_handle) {
        ov_curl_set_int(msg_handle, "groupID", mlp->group_id);
        if (ov_curl_json(msg_handle, ov_member_list_json_parse, mlp)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}

/**
 *  Create Group
 */

static void ov_create_group_json_parse(void *ov_msg_handle,
        jsmntok_t *tokens, char *data, void *user_data)
{
    jsmntok_t *t;

    if ((t = jsmn_tok(tokens, data, "["))) {
        ov_group_param_t *gp = (ov_group_param_t*)user_data;
        gp->id = jsmn_get_int(t, data, "GroupID");
    }
}

ov_msg_handle_t *ov_create_group(ov_handle_t *handle, char *group_name,
                                 ov_group_param_t *gp)
{
    ov_msg_handle_t *msg_handle = ov_curl_msg_init(handle, "createGroup", NULL,
                                  NULL);

    if (msg_handle) {
        ov_curl_set_string(msg_handle, "group", group_name);
        if (gp) {
            ov_curl_set_int(msg_handle, "autoLocation", gp->auto_location);
            ov_curl_set_int(msg_handle, "type", gp->type);
        }
        if (ov_curl_json(msg_handle, ov_create_group_json_parse, gp)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}

/**
 *  Delete group
 */

ov_msg_handle_t *ov_delete_group(ov_handle_t *handle, int id)
{
    ov_msg_handle_t *msg_handle = ov_curl_msg_init(handle, "deleteGroup", NULL,
                                  NULL);

    if (msg_handle) {
        ov_curl_set_int(msg_handle, "groupID", id);
        if (ov_curl_json(msg_handle, NULL, NULL)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}


int ov_group_keepalive(char *url, int *change_id)
{
    char *ptr;

    if ((ptr = ov_curl_get(url))) {
        *change_id = atoi(ptr);
        free(ptr);
    }
    return (ptr == NULL);
}

/**
 *  Get group details
 */

static void ov_get_group_details_json_parse(void *ov_msg_handle,
        jsmntok_t *tokens, char *data, void *user_data)
{
    jsmntok_t *t;

    if ((t = jsmn_tok(tokens, data, "{result"))) {
        ov_get_group_details_t *ggd = (ov_get_group_details_t*)user_data;
        ggd->id             = jsmn_get_int   (t, data, "Group_ID");
        ggd->name           = jsmn_get_string(t, data, "Group_Name");
        ggd->description    = jsmn_get_string(t, data, "Description");
        ggd->user_defined   = jsmn_get_string(t, data, "User_Defined");
        ggd->sent_msgs      = jsmn_get_int   (t, data, "Sent_Messages");
        ggd->received_msgs  = jsmn_get_int   (t, data, "Received_Messages");
        ggd->last_msg_id    = jsmn_get_int   (t, data, "Last_Message_ID");
        ggd->ip_addr        = jsmn_get_string(t, data, "IP_Address");
        ggd->member_id      = jsmn_get_int   (t, data, "Member_ID");
        if ((ggd->keepalive_url = jsmn_get_string(t, data, "Data_URL"))) {
            ov_trim(ggd->keepalive_url);
        }
    }
}

static void ov_get_group_details_json_cleanup(void *data)
{
    ov_get_group_details_t *gdd = data;

    free(gdd->name);
    free(gdd->description);
    free(gdd->ip_addr);
    free(gdd->user_defined);
    free(gdd->keepalive_url);
}

ov_msg_handle_t *ov_get_group_details(ov_handle_t *handle, char *group_name,
                                      ov_get_group_details_t *ggd)
{
    ov_msg_handle_t *msg_handle = ov_curl_msg_init(handle, "getGroupDetails",
                                  ov_get_group_details_json_cleanup, ggd);

    if (msg_handle) {
        ov_curl_set_string(msg_handle, "group", group_name);
        if (ov_curl_json(msg_handle, ov_get_group_details_json_parse, ggd)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}

/**
 *  Set group location
 */

ov_msg_handle_t *ov_set_group_location(ov_handle_t *handle,
                                       ov_group_location_param_t *glp)
{
    void *msg_handle = ov_curl_msg_init(handle, "setGroupLocation", NULL, NULL);

    if (msg_handle) {
        ov_curl_set_string(msg_handle, "group", glp->group_name);
        ov_curl_set_string(msg_handle, "longitude", glp->longitude);
        ov_curl_set_string(msg_handle, "latitude", glp->latitude);
        if (ov_curl_json(msg_handle, NULL, NULL)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}

/**
 *  Get group description
 */

static void ov_get_group_description_json_parse(void *ov_msg_handle,
        jsmntok_t *tokens, char *data, void *user_data)
{
    jsmntok_t *t;

    if ((t = jsmn_tok(tokens, data, "{result"))) {
        ov_group_description_param_t *gdp = (ov_group_description_param_t*)user_data;
        gdp->description      = jsmn_get_string(t, data, "Description");
    }
}

static void ov_get_group_description_json_cleanup(void *data)
{
    ov_group_description_param_t *gdp = data;

    free(gdp->description);
}

ov_msg_handle_t *ov_get_group_description(ov_handle_t *handle,
                                       ov_group_description_param_t *gdp)
{
    void *msg_handle = ov_curl_msg_init(handle, "getGroupDescription", ov_get_group_description_json_cleanup, gdp);

    if (msg_handle) {
        ov_curl_set_int(msg_handle, "groupID", gdp->group_id);
        if (ov_curl_json(msg_handle, ov_get_group_description_json_parse, gdp)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}

/**
 *  Set group description
 */

ov_msg_handle_t *ov_set_group_description(ov_handle_t *handle,
                                       ov_group_description_param_t *gdp)
{
    void *msg_handle = ov_curl_msg_init(handle, "setGroupDescription", NULL, NULL);

    if (msg_handle) {
        ov_curl_set_int(msg_handle, "groupID", gdp->group_id);
        ov_curl_set_string(msg_handle, "description", gdp->description);
        if (ov_curl_json(msg_handle, NULL, NULL)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}

/**
 *  Get group datastore
 */

static void ov_get_group_datastore_json_parse(void *ov_msg_handle,
        jsmntok_t *tokens, char *data, void *user_data)
{
    jsmntok_t *t;

    if ((t = jsmn_tok(tokens, data, "{result"))) {
        ov_group_datastore_param_t *gdp = (ov_group_datastore_param_t*)user_data;
        gdp->datastore      = jsmn_get_string(t, data, "Datastore");
        gdp->type = jsmn_get_int(t, data, "Datastore_Type");
    }
}

static void ov_get_group_datastore_json_cleanup(void *data)
{
    ov_group_datastore_param_t *gdp = data;

    free(gdp->datastore);
}

ov_msg_handle_t *ov_get_group_datastore(ov_handle_t *handle,
                                       ov_group_datastore_param_t *gdp)
{
    void *msg_handle = ov_curl_msg_init(handle, "getGroupDatastore", ov_get_group_datastore_json_cleanup, gdp);

    if (msg_handle) {
        ov_curl_set_int(msg_handle, "groupID", gdp->group_id);
        if (ov_curl_json(msg_handle, ov_get_group_datastore_json_parse, gdp)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}

/**
 *  Set group datastore
 */

ov_msg_handle_t *ov_set_group_datastore(ov_handle_t *handle,
                                       ov_group_datastore_param_t *gdp)
{
    void *msg_handle = ov_curl_msg_init(handle, "setGroupDatastore", NULL, NULL);

    if (msg_handle) {
        ov_curl_set_int(msg_handle, "groupID", gdp->group_id);
        ov_curl_set_string(msg_handle, "datastore", gdp->datastore);
        ov_curl_set_int(msg_handle, "datastoreType", gdp->type);
        if (ov_curl_json(msg_handle, NULL, NULL)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}

/**
 *  Create member
 */

static void ov_create_member_json_parse(void *ov_msg_handle, jsmntok_t *tokens,
                                       char *data, void *user_data)
{
    jsmntok_t *t;

    if ((t = jsmn_tok(tokens, data, "["))) {
        ov_member_param_t *mp = (ov_member_param_t*)user_data;
        mp->id             = jsmn_get_int(t, data, "MemberID");
    }
}

ov_msg_handle_t *ov_create_member(ov_handle_t *handle, ov_member_param_t *mp)
{
    void *msg_handle = ov_curl_msg_init(handle, "createMember", NULL, mp);

    if (msg_handle) {
        ov_curl_set_int(msg_handle, "groupID", mp->group_id);
        if (mp->type != OV_MEMBER_TYPE_DATA) {
            char first_period_last[80];
            snprintf(first_period_last, 79, "%s.%s", mp->first? mp->first : "first",
                     mp->last? mp->last : "last");
            ov_curl_set_string(msg_handle, "firstPeriodLast", first_period_last);
        }
        if (mp->first) {
            ov_curl_set_string(msg_handle, "first", mp->first);
        }
        if (mp->last) {
            ov_curl_set_string(msg_handle, "last", mp->last);
        }
        if (mp->phone) {
            ov_curl_set_string(msg_handle, "phone", mp->phone);
        }
        if (mp->email) {
            ov_curl_set_string(msg_handle, "email", mp->email);
        }
        if (mp->data_name) {
            ov_curl_set_string(msg_handle, "dataName", mp->data_name);
        }
        ov_curl_set_int(msg_handle, "type", mp->type);
        if (ov_curl_json(msg_handle, ov_create_member_json_parse, mp)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}

/**
 *  Delete member
 */

ov_msg_handle_t *ov_delete_member(ov_handle_t *handle, int group_id,
                                  int member_id)
{
    void *msg_handle = ov_curl_msg_init(handle, "deleteMember", NULL, NULL);

    if (msg_handle) {
        if (group_id) {
            /* group specified, remove from just this group */
            ov_curl_set_int(msg_handle, "groupID", group_id);
            ov_curl_set_int(msg_handle, "removeFlag", 0);
        } else {
            /* remove from all groups */
            ov_curl_set_int(msg_handle, "removeFlag", 1);
        }
        ov_curl_set_int(msg_handle, "memberRemoveID", member_id);
        if (ov_curl_json(msg_handle, NULL, NULL)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}

/**
 *  Set member datastore
 */

ov_msg_handle_t *ov_set_member_datastore(ov_handle_t *handle,
                                       ov_member_datastore_param_t *mdp)
{
    void *msg_handle = ov_curl_msg_init(handle, mdp->mode == APPEND? "setMemberAppendDatastore" : "setMemberDatastore", NULL, NULL);

    if (msg_handle) {
        ov_curl_set_string(msg_handle, "group", mdp->group_name);
        ov_curl_set_int(msg_handle, "memberID", mdp->member_id);
        ov_curl_set_string(msg_handle, "datastore", mdp->datastore);
        ov_curl_set_int(msg_handle, "datastoreType", mdp->type);
        if (ov_curl_json(msg_handle, NULL, NULL)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}

/**
 *  Create message
 */

#if NOT_YET /* Wait for return codes to get formalized */
static void ov_create_message_json_parse(void *ov_msg_handle, jsmntok_t *tokens,
        char *data, void *user_data)
{
    ov_create_message_t *cm = (ov_create_message_t*) user_data;

    cm->response.code = jsmn_get_int(tokens, data, "code");
    cm->response.success = jsmn_get_string(tokens, data, "success");
}

static void ov_create_message_json_cleanup(ov_create_message_t *cm)
{
    free(cm->response.success);
    cm->response.success = NULL;
}

#endif // NOT_YET

ov_msg_handle_t *ov_create_message(ov_handle_t*handle, ov_create_message_t *cm,
                                   char *message)
{
    void *msg_handle = ov_curl_msg_init(handle, "createMessage", NULL, NULL);

    if (msg_handle) {
        ov_curl_set_int(msg_handle, "groupID", cm->group_id);
        ov_curl_set_int(msg_handle, "notifyMembers", cm->notify_member);
        ov_curl_set_string(msg_handle, "message", message);
        if (ov_curl_json(msg_handle, NULL, NULL)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}

/**
 *  Delete message
 */

ov_msg_handle_t *ov_delete_message(ov_handle_t *handle, int message_id)
{
    void *msg_handle = ov_curl_msg_init(handle, "deleteMessage", NULL, NULL);

    if (msg_handle) {
        ov_curl_set_int(msg_handle, "messageID", message_id);
        if (ov_curl_json(msg_handle, NULL, NULL)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}

/**
 *  Get messages
 */

static void ov_get_messages_json_parse(void *ov_msg_handle, jsmntok_t *tokens,
                                       char *data, void *user_data)
{
    jsmntok_t *t;
	ov_get_message_list_t *gml = (ov_get_message_list_t*) user_data;

    if ((t = jsmn_tok(tokens, data, "{result"))) {
        int msg_id_max = 0;
        int count = t->size;
        ov_get_message_t *gm = gml->messages = count? calloc(count, sizeof(ov_get_message_t)) : NULL;
        gml->entry_count = count;
        t = jsmn_tok(t, data, "[");
        while (t && count--) {
            gm->message        = jsmn_get_string(t, data, "Message");
            gm->id             = jsmn_get_int(t, data, "Id");
            gm->member_id      = jsmn_get_int(t, data, "Member_ID");
            gm->added          = jsmn_get_string(t, data, "Added");
            gm->user           = jsmn_get_string(t, data, "User");
            gm->attach_number  = jsmn_get_int(t, data, "Attach_Number");
            gm->attach_storage = jsmn_get_string(t, data, "Attach_Storage");
            gm->attach_desc    = jsmn_get_string(t, data, "Attach_Desc");
            gm->first_name     = jsmn_get_string(t, data, "First_Name");
            gm->last_name      = jsmn_get_string(t, data, "Last_Name");
            gm->contact_means  = jsmn_get_int(t, data, "Contact_Means");
            if (msg_id_max < gm->id) {
                msg_id_max = gm->id;
            }
            gm++;
            t = jsmn_skip(t);
        }
        if (gml->msg_id_next < msg_id_max) {
            gml->msg_id_next = msg_id_max;
        }
    }
    else {
        gml->entry_count = 0;
    }
}

static void ov_get_messages_json_cleanup(void *data)
{
    ov_get_message_list_t *gml = data;
    if (gml->messages) {
        ov_get_message_t *gm = gml->messages;
        while (gml->entry_count--) {
            free(gm->message);
            free(gm->added);
            free(gm->user);
            free(gm->attach_storage);
            free(gm->attach_desc);
            free(gm->first_name);
            free(gm->last_name);
            gm++;
        }
        free(gml->messages);
        gml->messages = NULL;
    }
    gml->entry_count = 0;
}

ov_msg_handle_t *ov_get_messages(ov_handle_t *handle, int group_id,
                                 ov_get_message_list_t *gml)
{
   void *msg_handle = ov_curl_msg_init(handle, "getMessagesByID",
                                         ov_get_messages_json_cleanup, gml);

    if (msg_handle) {
        ov_curl_set_int(msg_handle, "groupID", group_id);
        ov_curl_set_int(msg_handle, "id", gml->msg_id_next);
        if (gml->entry_count <= 0) {
            gml->entry_count = OV_GET_MESSAGES_BY_ID_DEFAULT_COUNT;
        }
        ov_curl_set_int(msg_handle, "count", gml->entry_count);
        if (ov_curl_json(msg_handle, ov_get_messages_json_parse, gml)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}

/**
 *  Get file list
 */

static void ov_get_file_list_json_parse(void *ov_msg_handle, jsmntok_t *tokens,
                                        char *data, void *user_data)
{
    jsmntok_t *t;

    if ((t = jsmn_tok(tokens, data, "{result"))) {
        int count = t->size;
        ov_file_list_param_t *gfl = (ov_file_list_param_t*) user_data;
        ov_file_list_status_t *gf = gfl->entries = calloc(count,
                                    sizeof(ov_file_list_status_t));
        gfl->entry_count = count;
        t = jsmn_tok(t, data, "[");
        while (t && count--) {
            gf->id             = jsmn_get_int(t, data, "File_ID");
            gf->date_created   = jsmn_get_string(t, data, "Added_Date");
            gf->name           = jsmn_get_string(t, data, "File_Name");
            gf->extension      = jsmn_get_string(t, data, "Extension");
            gf->member_id      = jsmn_get_int(t, data, "Member_ID");
            gf->storage_name   = jsmn_get_string(t, data, "Storage_Name");
            gf++;
            t = jsmn_skip(t);
        }
    }
}

static void ov_get_file_list_json_cleanup(void *data)
{
    ov_file_list_param_t *gfl = data;
    if (gfl->entries) {
        ov_file_list_status_t *gf = gfl->entries;
        while (gfl->entry_count--) {
            free(gf->date_created);
            free(gf->name);
            free(gf->extension);
            free(gf->storage_name);
            gf++;
        }
        free(gfl->entries);
        gfl->entries = NULL;
    }
    gfl->entry_count = 0;
}

ov_msg_handle_t *ov_get_file_list(ov_handle_t *handle, int group_id,
                                  ov_file_list_param_t *flp)
{
    void *msg_handle = ov_curl_msg_init(handle, "getFileList",
                                        ov_get_file_list_json_cleanup, flp);

    if (msg_handle) {
        ov_curl_set_int(msg_handle, "groupID", group_id);
        if (ov_curl_json(msg_handle, ov_get_file_list_json_parse, flp)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}

/**
 *  Put file
 */

ov_msg_handle_t *ov_put_file(ov_handle_t *handle, int group_id,
                             ov_put_file_param_t *pfp)
{
    void *msg_handle;
    struct stat file_info;

    /* get the file size of the local file */
    if (stat(pfp->name, &file_info)) {
        /* file error, bail */
        return NULL;
    }

    if ((msg_handle = ov_curl_msg_init(handle, "uploadFile", NULL, pfp))) {
        ov_curl_set_int(msg_handle, "groupID", group_id);
        ov_curl_set_int(msg_handle, "notifyMembers", pfp->notify_member);
        ov_curl_set_int(msg_handle, "filesize", file_info.st_size);
        ov_curl_set_string(msg_handle, "title", pfp->title);
        ov_curl_set_string(msg_handle, "name", pfp->name);
        ov_curl_set_filedata(msg_handle, "file", pfp->name);
        if (ov_curl_json(msg_handle, NULL, pfp)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}

/**
 *  Get file
 */

static void ov_get_file_json_parse(void *ov_msg_handle, jsmntok_t *tokens,
                                   char *data, void *user_data)
{
    ov_get_file_param_t *gfp = (ov_get_file_param_t*) user_data;

    gfp->url = jsmn_get_string(tokens, data, "File");
    gfp->id = jsmn_get_int(tokens, data, "File_ID");
}

static void ov_get_file_json_cleanup(void *data)
{
    ov_get_file_param_t *gfp = (ov_get_file_param_t*)data;

    if (gfp->url) free(gfp->url);
}

ov_msg_handle_t *ov_get_file(ov_handle_t *handle, int group_id,
                             ov_get_file_param_t *gfp)
{
    void *msg_handle;

    if ((msg_handle = ov_curl_msg_init(handle, "getFile", ov_get_file_json_cleanup,
                                       gfp))) {
        ov_curl_set_int(msg_handle, "groupID", group_id);
        ov_curl_set_string(msg_handle, "fileID", "0");
        ov_curl_set_string(msg_handle, "fileName", gfp->name);
        if ((ov_curl_json(msg_handle, ov_get_file_json_parse, gfp)
                || gfp->url == NULL || gfp->id == 0)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        } else {
            /* we've retrieved the file URL, now download the file */
            ov_trim(gfp->url);
            ov_curl_download(handle, gfp->local_path, gfp->url);
        }
    }
    return msg_handle;
}

/**
 *  Delete file
 */

ov_msg_handle_t *ov_delete_file(ov_handle_t *handle, int group_id,
                                ov_delete_file_param_t *dfp)
{
    // TODO
    return NULL;
}

/**
 *  Create rule
 */

ov_msg_handle_t *ov_rule_create(ov_handle_t *handle, int group_id,
                                ov_rule_t *rule)
{
    void *msg_handle;

    if ((msg_handle = ov_curl_msg_init(handle, "createRule", NULL, NULL))) {
        ov_curl_set_int(msg_handle, "groupID", group_id);
        ov_curl_set_string(msg_handle, "name", rule->name);
        ov_curl_set_string(msg_handle, "description", rule->description);
        ov_curl_set_int(msg_handle, "active", rule->active? 2 : 1);
        ov_curl_set_int(msg_handle, "rInt_1", rule->direction);
        /* set up the event */
        ov_curl_set_int(msg_handle, "ruleType", rule->event->type);
        switch (rule->event->type) {
        case TYPE_TRIGGER:
            ov_curl_set_string(msg_handle, "rStr_1", rule->event->u.trigger.search_text);
            break;
        case TYPE_COUNT:
            ov_curl_set_int(msg_handle, "rInt_8", rule->event->u.count.limit);
            ov_curl_set_int(msg_handle, "rInt_9", rule->event->u.count.interval);
            break;
        default:
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            return NULL;
        }
        /* set up the action */
        switch (rule->action->type) {
        case TELL_MEMBER:
            ov_curl_set_string(msg_handle, "rStr_2", rule->action->u.member.name);
            /* TODO: set rInt_7 to member ID? */
            break;
        case TELL_GROUP: {
            /* make sure this group exists first, and get the group ID */
            char *group_to_tell = rule->action->u.group.name;
            ov_get_group_details_t ggd = {};
            ov_msg_handle_t *msg_handle2 = ov_get_group_details(handle, group_to_tell,
                                           &ggd);
            if (msg_handle2) {
                /* success */
                ov_curl_set_int(msg_handle, "rInt_7", ggd.id);
                ov_msg_cleanup(msg_handle2);
                ov_curl_set_string(msg_handle, "rStr_2", group_to_tell);
            } else {
                printf("get group details for %s failed\n", group_to_tell);
                ov_msg_cleanup(msg_handle);
                return NULL;
            }
            break;
        }
        case SEND_TO_URL:
            ov_curl_set_string(msg_handle, "rStr_2", rule->action->u.url.name);
            break;
        default:
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            return NULL;
        }
        ov_curl_set_int(msg_handle, "rInt_2", (int)rule->action->type);
        ov_curl_set_int(msg_handle, "rInt_3", rule->action->include_member_id);
        ov_curl_set_int(msg_handle, "rInt_4", rule->action->include_group_id);
        ov_curl_set_int(msg_handle, "rInt_5", rule->action->include_original_message);
        ov_curl_set_int(msg_handle, "rInt_6", rule->action->new_message? 1 : 0);
        if (rule->action->new_message) {
            ov_curl_set_string(msg_handle, "rStr_3", rule->action->new_message);
        }
        /* send it off */
        if (ov_curl_json(msg_handle, NULL, NULL)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}

/**
 *  Delete rule
 */

ov_msg_handle_t *ov_delete_rule(ov_handle_t *handle, int group_id, int rule_id)
{
    void *msg_handle = ov_curl_msg_init(handle, "deleteRule", NULL, NULL);

    if (msg_handle) {
        ov_curl_set_int(msg_handle, "groupID", group_id);
        ov_curl_set_int(msg_handle, "ruleID", rule_id);
        if (ov_curl_json(msg_handle, NULL, NULL)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}

/**
 *  Get rule list
 */

static void ov_get_rules_json_parse(void *ov_msg_handle, jsmntok_t *tokens,
                                    char *data, void *user_data)
{
    jsmntok_t *t;

    if ((t = jsmn_tok(tokens, data, "{result"))) {
        int count = t->size;
        ov_get_rule_list_t *grl = (ov_get_rule_list_t*) user_data;
        ov_get_rule_t *gr = grl->rules = calloc(count, sizeof(ov_get_rule_t));
        ov_rule_t *r;
        ov_rule_event_t *re;
        ov_rule_action_t *ra;
        grl->entry_count = count;
        t = jsmn_tok(t, data, "[");
        while (t && count--) {
            gr->id = jsmn_get_int(t, data, "Rule_ID");
            gr->date_created = jsmn_get_string(t, data, "Date_Created");
            r = &gr->rule;
            r->event = re = &gr->event;
            r->action = ra = &gr->action;
            r->active = jsmn_get_int(t, data, "Active");
            r->direction = jsmn_get_int(t, data, "RINT_1");
            r->name = jsmn_get_string(t, data, "Name");
            r->description = jsmn_get_string(t, data, "Description");
            /* process event info */
            switch ((re->type = jsmn_get_int(t, data, "Type"))) {
            case TYPE_TRIGGER:
                re->u.trigger.search_text = jsmn_get_string(t, data, "RSTR_1");
                break;
            case TYPE_COUNT:
                re->u.count.limit = jsmn_get_int(t, data, "RINT_8");
                re->u.count.interval = jsmn_get_int(t, data, "RINT_9");
                break;
            default:
                /* error, skip */
                gr->err = 1;
                goto next;
            }
            /* process action info */
            switch ((ra->type = jsmn_get_int(t, data, "RINT_2"))) {
            case TELL_MEMBER:
                ra->u.member.name = jsmn_get_string(t, data, "RSTR_2");
                break;
            case TELL_GROUP:
                ra->u.group.name = jsmn_get_string(t, data, "RSTR_2");
                /* Get group ID? */
                break;
            case SEND_TO_URL:
                ra->u.url.name = jsmn_get_string(t, data, "RSTR_2");
                break;
            default:
                /* error, skip */
                gr->err = 1;
                goto next;
            }
            ra->include_member_id  = jsmn_get_int(t, data, "RINT_3");
            ra->include_group_id  = jsmn_get_int(t, data, "RINT_4");
            ra->include_original_message  = jsmn_get_int(t, data, "RINT_5");
            ra->include_member_id  = jsmn_get_int(t, data, "RINT_3");
            if (jsmn_get_int(t, data, "RINT_6")) {
                ra->new_message = jsmn_get_string(t, data, "RSTR_3");
            }
next:
            gr++;
            t = jsmn_skip(t);
        }
    }
}

static void ov_get_rules_json_cleanup(void *data)
{
    ov_get_rule_list_t *grl = data;

    if (grl->rules) {
        ov_rule_t *r;
        ov_get_rule_t *gr = grl->rules;
        while (grl->entry_count--) {
            ov_rule_event_t *re;
            ov_rule_action_t *ra;
            free(gr->date_created);
            r = &gr->rule;
            re = r->event;
            ra = r->action;
            free(r->name);
            free(r->description);
            switch (re->type) {
            case TYPE_TRIGGER:
                free(re->u.trigger.search_text);
                break;
            case TYPE_COUNT:
                break;
            default:
                /* error, keep going */
                break;
            }
            switch (ra->type) {
            case TELL_MEMBER:
                free(ra->u.member.name);
                break;
            case TELL_GROUP:
                free(ra->u.group.name);
                break;
            case SEND_TO_URL:
                free(ra->u.url.name);
                break;
            default:
                /* error, keep going */
                break;
            }
            free(ra->new_message);
            gr++;
        }
        free(grl->rules);
        grl->rules = NULL;
    }
    grl->entry_count = 0;
}

ov_msg_handle_t *ov_get_rules(ov_handle_t *handle, int group_id,
                              ov_get_rule_list_t *grl)
{
    void *msg_handle = ov_curl_msg_init(handle, "getRules",
                                        ov_get_rules_json_cleanup, grl);

    if (msg_handle) {
        ov_curl_set_int(msg_handle, "groupID", group_id);
        if (ov_curl_json(msg_handle, ov_get_rules_json_parse, grl)) {
            /* error */
            ov_curl_msg_cleanup(msg_handle);
            msg_handle = NULL;
        }
    }
    return msg_handle;
}
