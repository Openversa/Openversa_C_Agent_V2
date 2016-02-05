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

#ifndef __OV_API__
#define __OV_API__

#define OV_C_API_VERSION_ID "1.3"

typedef void ov_handle_t;
typedef void ov_msg_handle_t;

typedef enum {
    OV_ERR_OK,
} OV_ERR;

/* Utility functions */
void ov_trim(char *buf);

/* Init */
typedef struct {
    struct timeval response_time;
    time_t timestamp;
} ov_stats_entry_t;

typedef struct {
    int debug;
    int get_stats;
} ov_param_t;

ov_handle_t *ov_init(const char *app_id, const char *app_secret,
                     const char *user_name, const char *url, ov_param_t*);
void ov_msg_cleanup(ov_msg_handle_t*);

/* Shutdown */
void ov_shutdown(ov_handle_t*);

/* Report ov stats */
int ov_report_stats(ov_handle_t *ov_handle, char *group_name);

/* Create group */
typedef struct {
    int auto_location;
    int type;
    int id;                 // returned group id
} ov_group_param_t;

ov_msg_handle_t *ov_create_group(ov_handle_t*, char *group_name,
                                 ov_group_param_t*);

/* Delete group */
ov_msg_handle_t *ov_delete_group(ov_handle_t*, int id);

/* Get group details */
typedef struct {
    int id;
    char *name;
    char *description;
    char *user_defined;
    char *ip_addr;
    char *keepalive_url;
    int sent_msgs;
    int received_msgs;
    int last_msg_id;
    int member_id;
} ov_get_group_details_t;

ov_msg_handle_t *ov_get_group_details(ov_handle_t*, char *group_name,
                                      ov_get_group_details_t*);

int ov_group_keepalive(char *url, int *change_id);

int ov_group_name_to_id(ov_handle_t *ov_handle, char *group_name);

/* Set group location */
typedef struct {
    char *group_name;
    char *latitude;
    char *longitude;
} ov_group_location_param_t;

ov_msg_handle_t *ov_set_group_location(ov_handle_t *handle,
                                       ov_group_location_param_t*);

/* Set group description */
typedef struct {
    int group_id;
    char *description;
} ov_group_description_param_t;

ov_msg_handle_t *ov_set_group_description(ov_handle_t *handle,
                                       ov_group_description_param_t*);

ov_msg_handle_t *ov_get_group_description(ov_handle_t *handle,
                                       ov_group_description_param_t*);

/* Set group datastore */
typedef struct {
    int group_id;
    char *datastore;
    enum {TEXT = 0, JSON = 1, CSV = 2 } type;
} ov_group_datastore_param_t;

ov_msg_handle_t *ov_set_group_datastore(ov_handle_t *handle,
                                       ov_group_datastore_param_t*);

ov_msg_handle_t *ov_get_group_datastore(ov_handle_t *handle,
                                       ov_group_datastore_param_t*);

/* Get [shared] group list */
typedef struct {
    char *date_created;
    char *name;
    int id;
    int avatar;
    int scheduled_count;
    int chat_count;
} ov_group_list_status_t;

typedef struct {
    int entry_count;
    ov_group_list_status_t *entries;
    int do_badges;
} ov_group_list_param_t;

ov_msg_handle_t *ov_get_group_list(ov_handle_t*, ov_group_list_param_t*);
ov_msg_handle_t *ov_get_shared_group_list(ov_handle_t*, ov_group_list_param_t*);

/* Delete group list */
ov_msg_handle_t *ov_delete_group_list(ov_handle_t*, int count, int *entries);

/* Create member */
typedef enum {
    OV_MEMBER_TYPE_FACEBOOK = 1,
    OV_MEMBER_TYPE_EMAIL,
    OV_MEMBER_TYPE_SMS,
    OV_MEMBER_TYPE_TWITTER,
    OV_MEMBER_TYPE_DATA = 9,
    OV_MEMBER_TYPE_MAX              // Last entry
} OV_MEMBER_TYPE;

typedef struct {
    char *token;                // contact preference type social OAth token
    char *secret;               // contact preference type social OAth secret
} oauth_t;

typedef struct {
    int group_id;
    char *first;                    // first name
    char *last;                     // last name
    char *phone;                    // US phone number no special characters or white space (if type SMS)
    char *email;                    // email of the person (only if the type is email)
    char *data_name;                // data member name (only if the type is data)
    OV_MEMBER_TYPE type;            // contact preference type
    oauth_t *oauth;                 // contact preference type social OAth info
    char *device;                   // device ID of the mobile phone or device
    int id;                         // member id (returned on creation)
} ov_member_param_t;

ov_msg_handle_t *ov_create_member(ov_handle_t*, ov_member_param_t*);

/* Delete member */
typedef enum {
    OV_MEMBER_DELETE_FROM_THIS_GROUP,
    OV_MEMBER_DELETE_FROM_ALL_GROUPS,
} OV_MEMBER_DELETE_MODE;

ov_msg_handle_t *ov_delete_member(ov_handle_t*, int group_id, int member_id);

/* Delete member list -- TODO */

/* Get members */
typedef struct {
    char *name;
    int id;
    char *phone_mobile;
    char *email;
    char *first_name;
    char *last_name;
    OV_MEMBER_TYPE contact_means;
} ov_member_status_t;

typedef struct {
    int entry_count;
    ov_member_status_t *entries;
    int group_id;
} ov_member_list_t;

ov_msg_handle_t *ov_get_member_list(ov_handle_t*, ov_member_list_t*);

/* Get member details */
ov_msg_handle_t *ov_get_member_details(ov_handle_t*, int group_id, int id,
                                       ov_member_status_t*);

/* Set member details */
ov_msg_handle_t *ov_set_member_details(ov_handle_t*, int group_id, int id,
                                       ov_member_param_t*);

/* Set member datastore */
typedef enum {
    OV_MEMBER_DATASTORE_TYPE_NONE = 0,
    OV_MEMBER_DATASTORE_TYPE_JSON = 1,
    OV_MEMBER_DATASTORE_TYPE_CSV = 3,
    OV_MEMBER_DATASTORE_TYPE_MAX              // Last entry
} OV_MEMBER_DATASTORE_TYPE;


typedef struct {
    char *group_name;
    int member_id;
    char *datastore;
    OV_MEMBER_DATASTORE_TYPE type;
    enum {
        REPLACE = 0,
        APPEND
    } mode;
} ov_member_datastore_param_t;

ov_msg_handle_t *ov_set_member_datastore(ov_handle_t *handle,
                                       ov_member_datastore_param_t*);

/* Set member mobile number  -- TODO */

/* Set member contact type  -- TODO */

/* Get members timezone  -- TODO */

/* Create/Send message */
typedef struct {
    int group_id;
    int notify_member;     // flag to send the message to the members
    char *schedule;        // (optional) time if this message is to be sent on a schedule “2014-05-14 05:16:28”
    char *file_id;         // (optional) file ID of the file that was uploaded prior to OpenVersa
    int reply_id;          // (optional) ID of the message if this is a reply to a particular incoming message
    int direct_id;         // (optional) ID of the member you want to send a direct message to (not the entire group)
} ov_create_message_t;

ov_msg_handle_t *ov_create_message(ov_handle_t*, ov_create_message_t*,
                                   char *message);

/* Delete message */
ov_msg_handle_t *ov_delete_message(ov_handle_t*, int message_id);

/* Get messages [by time]|[by ID] */
typedef struct {
    int shared_group_id;         // (optional) is the group ID if shared.
    int group_id;                // (optional) is the group ID.
    time_t time;                 // time from which messages will be received (0 = all messages)
    int id;                  // the ID of the message that all messages newer will be retrieved
    int count;                  // (optional) max number of messages to be received
} ov_get_message_req_t;

typedef struct {
    char *message;               // the message in text
    int id;                    // message ID
    int member_id;             // the member ID of originator of the message
    char *added;                 // UTC time code of the create date
    char *user;                  // email of registered user of message creator
    int  attach_number;         // file ID number if attached
    char *attach_desc;           // name of the file if attached
    char *attach_storage;        // hash location of file in storage if attached to message
    char *first_name;            // the first name of the message creator
    char *last_name;             // the last name of the message creator
    OV_MEMBER_TYPE contact_means; // current type of the message creator
} ov_get_message_t;

typedef struct {
    int entry_count;
    ov_get_message_t *messages;
    int shared_group_id;         // (optional) is the group ID if shared.
    int group_id;                // (optional) is the group ID.
    time_t time;                 // time from which messages will be received (0 = all messages)
    int msg_id_next;			 // returned last message id
} ov_get_message_list_t;

ov_msg_handle_t *ov_get_messages(ov_handle_t*, int group_id,
                                 ov_get_message_list_t*);

/* Get file list */
typedef struct {
    char *date_created;
    char *name;
    char *extension;
    int id;
    char *storage_name;
    int size;
    int member_id;
} ov_file_list_status_t;

typedef struct {
    int entry_count;
    ov_file_list_status_t *entries;
} ov_file_list_param_t;

ov_msg_handle_t *ov_get_file_list(ov_handle_t*, int group_id,
                                  ov_file_list_param_t*);

/* Put file */

typedef struct {
    int notify_member;     // flag to notify the members
    char *name;            // filename (incl. extension)
    char *title;            // filename (incl. extension)
} ov_put_file_param_t;

ov_msg_handle_t *ov_put_file(ov_handle_t *handle, int group_id,
                             ov_put_file_param_t*);

/* Get file */

typedef struct {
    char *name;            // filename (incl. extension)
    char *local_path;      // local filepath (incl. extension)
    int id;                // returned file ID )
    char *url;             // returned URL for downloading )
} ov_get_file_param_t;

ov_msg_handle_t *ov_get_file(ov_handle_t *handle, int group_id,
                             ov_get_file_param_t*);

/* Delete file */

typedef struct {
    int notify_member;     // flag to notify the members
    char *name;            // filename (incl. extension)
    char *title;            // filename (incl. extension)
} ov_delete_file_param_t;

ov_msg_handle_t *ov_delete_file(ov_handle_t *handle, int group_id,
                                ov_delete_file_param_t*);

/* Create rule */

typedef struct {
    enum {
        TYPE_TRIGGER = 1,
        TYPE_COUNT = 2,
    } type;
    union {
        struct {
            char *search_text;  // string which will be found within the message causing the trigger to execute
        } trigger;
        struct {
            int limit; // the count of occurnaces to activate the count rule
            int interval; // the number of seconds representing the time cap for the count rule occurances to be within.
        } count;
    } u;
} ov_rule_event_t;

typedef struct {
    enum {
        TELL_MEMBER = 1,
        TELL_GROUP = 2,
        SEND_TO_URL = 3
    } type;
    union {
        struct {
            char *name;
        } member;
        struct {
            char *name;
        } group;
        struct {
            char *name;
        } url;
    } u;
    int include_member_id;
    int include_group_id;
    int include_original_message;
    char *new_message;
} ov_rule_action_t;

typedef struct {
    char *name;             // (optional) name of rule
    char *description;      // (optional) description of the rule
    int active;             // integer flag for active or not, 1=rule (ON), 0=rule (OFF)
    enum {
        DIR_SEND = 1,
        DIR_RECEIVE = 2
    } direction; // flag to trigger on 1=the sending a message, 2=the receiving a message
    ov_rule_event_t *event; // rule triggering event
    ov_rule_action_t *action; // rule action
} ov_rule_t;

ov_msg_handle_t *ov_rule_create(ov_handle_t*, int group_id, ov_rule_t*);

/* Delete rules */

ov_msg_handle_t *ov_delete_rule(ov_handle_t*, int group_id, int rule_id);

/* Get rules */

typedef struct {
    int id;			// Rule ID
    int err;
    ov_rule_t rule;
    char *date_created;
    ov_rule_event_t event;
    ov_rule_action_t action;
} ov_get_rule_t;

typedef struct {
    int entry_count;
    ov_get_rule_t *rules;
    int group_id;                // (optional) is the group ID.
} ov_get_rule_list_t;

ov_msg_handle_t *ov_get_rules(ov_handle_t*, int group_id, ov_get_rule_list_t*);

#endif
