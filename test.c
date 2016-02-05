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
#include <string.h>
#include <unistd.h>

#include "ovlib/ov_api.h"

/*
 * Print out the groups
 */

static int demo_print_group_list(ov_handle_t *ov_handle, int do_badges)
{
    ov_group_list_param_t gp =
    {
        .entry_count = -1,
        .do_badges = do_badges
    };
    ov_msg_handle_t *msg_handle = ov_get_group_list(ov_handle, &gp);

    if (msg_handle)
    {
        ov_group_list_status_t *ge;
        int i;
        printf("Received %d groups\n", gp.entry_count);
        printf("+----------------------+-------------------+------+------+\n");
        printf("|Group Name            |Date Created       |ID    |Chat #|\n");
        printf("+----------------------+-------------------+------+------+\n");
        for (i = 0, ge = gp.entries; i < gp.entry_count; i++, ge++)
        {
            printf("|%-22s|%19s|%6d|%6d|\n", ge->name, ge->date_created, ge->id, ge->chat_count);
        }
        printf("+----------------------+-------------------+------+------+\n");
        ov_msg_cleanup(msg_handle);
    }
    else
    {
        printf("OpenVersa group get failed\n");
    }
    return msg_handle ? 0 : 1;
}

/*
 * Print out the members in a group
 */

static int demo_print_member_list(ov_handle_t *ov_handle, int group_id)
{
    ov_member_list_t mp =
    {
        .entry_count = -1,
        .group_id = group_id
    };
    int i;
    ov_msg_handle_t *msg_handle = ov_get_member_list(ov_handle, &mp);

    if (msg_handle)
    {
        ov_member_status_t *me;
        printf("+--------------------------------------------------------------------------+\n");
        printf("|     Members In Group %-40d            |\n", group_id);
        printf("+--------------------------------------+------+-+--------------------------+\n");
        printf("|Name                                  |ID    |C|Email                     |\n");
        printf("+--------------------------------------+------+-+--------------------------+\n");
        for (i = 0, me = mp.entries; i < mp.entry_count; i++, me++)
        {
            printf("|%-38s|%6d|%d|%-26s|\n", me->name, me->id, me->contact_means, me->email);
        }
        printf("+--------------------------------------+------+-+--------------------------+\n");
        ov_msg_cleanup(msg_handle);
    }
    else
    {
        printf("OpenVersa member get failed for group %d\n", group_id);
    }
    return msg_handle ? 0 : 1;
}

/*
 * Print out the messages sent to a group
 */

static int demo_print_messages(ov_handle_t *ov_handle, int group_id)
{
    ov_get_message_t *ms;
    ov_get_message_list_t mp =
    {
        .group_id = group_id
    };
    int i;
    ov_msg_handle_t *msg_handle = ov_get_messages(ov_handle, group_id, &mp);

    if (msg_handle)
    {
        printf("+-----------------------------------------------------------------------------+\n");
        printf("|     Messages to Group %-40d              |\n", group_id);
        printf("+------+--------------------------+-------------------------------------------+\n");
        printf("|Msg ID|Added              |From  |Message                                    |\n");
        printf("+------+-------------------+------+-------------------------------------------+\n");
        for (i = 0, ms = mp.messages; i < mp.entry_count; i++, ms++)
        {
            printf("|%6d|%19s|%6d|%-43s|\n", ms->id, ms->added, ms->member_id, ms->message);
        }
        printf("+------+-------------------+------+-------------------------------------------+\n");
        ov_msg_cleanup(msg_handle);
    }
    else
    {
        printf("OpenVersa message get failed for group %d\n", group_id);
    }
    return msg_handle ? 0 : 1;
}

/*
 * Print out a group's file list
 */

static int demo_print_file_list(ov_handle_t *ov_handle, int group_id)
{
    ov_file_list_param_t flp = {};
    int i;
    ov_msg_handle_t *msg_handle = ov_get_file_list(ov_handle, group_id, &flp);
    if (msg_handle)
    {
        ov_file_list_status_t *fs;
        printf("+------+--------------------------+-------------------------------------------+-------------------------------------------+\n");
        printf("|     Files in Group %-40d                                                             |\n", group_id);
        printf("+------+--------------------------+-------------------------------------------+-------------------------------------------+\n");
        printf("|FileID|Added              |From  |Name                                       |Storage Name                               |\n");
        printf("+------+--------------------------+-------------------------------------------+-------------------------------------------+\n");
        for (i = 0, fs = flp.entries; i < flp.entry_count; i++, fs++)
        {
            printf("|%6d|%19s|%6d|%-43s|%-43s|\n", fs->id, fs->date_created, fs->member_id, fs->name, fs->storage_name);
        }
        printf("+------+-------------------+------+-------------------------------------------+\n");
        ov_msg_cleanup(msg_handle);
    }
    else
    {
        printf("OpenVersa file list get failed for group %d\n", group_id);
    }
    return msg_handle ? 0 : 1;
}

/*
 * Print out the rules configured in a group
 */

static int demo_print_rules(ov_handle_t *ov_handle, int group_id)
{
    ov_get_rule_t *gr;
    ov_get_rule_list_t grl =
    {
        .entry_count = -1,
        .group_id = group_id
    };
    int i;
    ov_msg_handle_t *msg_handle;

    if (grl.group_id == -1)
    {
        return 1;
    }

    if ((msg_handle = ov_get_rules(ov_handle, group_id, &grl)))
    {
        ov_rule_t *r;
        ov_rule_event_t *re;
        ov_rule_action_t *ra;

        printf("+-----------------------------------------------------------------------------+\n");
        printf("|     %3d Rules for Group %-40d            |\n", grl.entry_count, group_id);
        printf("+------+----------------------------------------------------------------------+\n");
        printf("|RuleID|Content                                                               |\n");
        printf("+------+----------------------------------------------------------------------+\n");

        for (i = 0, gr = grl.rules; i < grl.entry_count; i++, gr++)
        {
            r = &gr->rule;
            re = r->event;
            ra = r->action;
            printf("|%6d| Name:        %-30s  Active = %1d Dir = %s|\n", gr->id, r->name, r->active,
                   r->direction == DIR_SEND? "Send   " : "Receive");
            printf("|      | Description: %-56s|\n", r->description);
            switch (re->type)
            {
            case TYPE_TRIGGER:
                printf("|      | Type: Trigger, Match String: %-40s|\n", re->u.trigger.search_text);
                break;
            case TYPE_COUNT:
                printf("|      | Type: Count, Limit = %3d, Interval = %5d                           |\n", re->u.count.limit, re->u.count.interval);
                break;
            default:
                break;
            }
            switch (ra->type)
            {
            case TELL_MEMBER:
                printf("|      | Tell Member: %-34s", ra->u.member.name);
                break;
            case TELL_GROUP:
            {
                printf("|      | Tell Group:  %-34s", ra->u.group.name);
                break;
            }
            case SEND_TO_URL:
                printf("|      | Tell URL:    %-34s|", ra->u.url.name);
                break;
            default:
                printf("|      | Action not recognized: %10d|", ra->type);
                break;
            }
            printf(" Include [%s,%s,%s,%s]|\n",
                   ra->include_member_id? "MI" : "--",
                   ra->include_group_id? "GI" : "--",
                   ra->include_original_message? "OM" : "--",
                   ra->new_message? "NM" : "--");
            if (ra->new_message)
            {
                printf("|      | New Message: %-56s|\n", ra->new_message);
            }
            printf("+------+----------------------------------------------------------------------+\n");
        }
        ov_msg_cleanup(msg_handle);
    }
    else
    {
        printf("OpenVersa rules get failed for group %d\n", group_id);
    }
    return msg_handle ? 0 : 1;
}

static const char *test_step_desc[] =
{
    " 1: Get group list",
    " 2: Create new group 'OV_TEST_ABC'",
    " 3: Get group list again, has new group been added?",
    " 4: Delete group 'OV_TEST_ABC'",
    " 5: Get group list again, group should have gone",
    " 6: Create new group 'OV_TEST_ABC' again",
    " 7: Show all members of all groups",
    " 8: Add a member to 'OV_TEST_ABC'",
    " 9: Show all members of 'OV_TEST_ABC'; 'Sally Jones' should have been added",
    "10: Delete 'Sally Jones' from group 'OV_TEST_ABC'",
    "11: Show all members of 'OV_TEST_ABC'; 'Sally Jones' should have been removed",
    "12: Send 3 messages to 'OV_TEST_ABC'",
    "13: Get messages sent to 'OV_TEST_ABC' - Should see 'OpenVersa API Test Message'",
    "14: Remove all messages from the group 'OV_TEST_ABC'",
    "15: Get messages sent to 'OV_TEST_ABC'. Should be empty",
    "16: Get file list for group 'OV_TEST_ABC'. Should be empty",
    "17: Upload file 'abc.txt' group 'OV_TEST_ABC'",
    "18: Get file list for group 'OV_TEST_ABC', is 'abc.txt' there?",
    "19: Download file 'abc.txt' group 'OV_TEST_ABC'",
    "20: Show rules for group 'OV_TEST_ABC'. Should be empty",
    "21: Add Group 'OV_Test_GHI' and 2 rules",
    "22: Get rule list for Group 'OV_TEST_ABC', Are 2 rules there?",
    "23: Delete Group 2 rules and 'OV_Test_GHI'",
    "24: Show rules for group 'OV_TEST_ABC'. Should be empty",
    "25: Get Description for group'OV_TEST_ABC', should be empty",
    "26: Set Description for group'OV_TEST_ABC' to 'lkjhgfdsa'",
    "27: Get Description for group'OV_TEST_ABC', should be 'lkjhgfdsa'",
    "28: Get Data store for group'OV_TEST_ABC', should be empty",
    "29: Set Data store for group'OV_TEST_ABC' to 'qwertyuiop'",
    "30: Get Data store for group'OV_TEST_ABC', should be 'qwertyuiop",
#if NOT_YET
    "31: Show all data members of 'OV_TEST_ABC'",
    "32: Add 10 data members to 'OV_TEST_ABC'",
    "33: Show data members of 'OV_TEST_ABC'; 10 should have been added",
    "34: Delete group 'OV_TEST_ABC'",
    "35: Get group list again, group should have gone",
    "36: The End!",
#else
    "31: Delete group 'OV_TEST_ABC'",
    "32: Get group list again, group should have gone",
    "33: The End!",
#endif
};

int ov_main_test(ov_handle_t *ov_handle, int argc, char *argv[])
{
    ov_msg_handle_t *msg_handle, *msg_handle2;
    int rc = 0, test_no = 1, i;
    ov_group_param_t gp = { .auto_location = 1 };
    char *test_group_name = "OV_TEST_ABC";
    int test_group;
    int test_no_start_man = 1;

    if (argc == 2)
    {
        /* start manual test case specified */
        if ((test_no_start_man = atoi(argv[1])) < 1)
        {
            test_no_start_man = 1;
        }
    }

    while (rc == 0)
    {
        printf("======================== TEST STEP %s =====================\n", test_step_desc[test_no - 1]);

        if (test_no >= test_no_start_man)
        {
            /* Uncomment to pause for each step */
            printf("press a key to continue...");
            if (getchar() == 'f')
            {
                /* finish test automatically */
                test_no_start_man = 100;
            };
            printf("\n");
        }
        switch (test_no++)
        {
        case 1: /* get group list */
            rc = demo_print_group_list(ov_handle, 0);
            break;
        case 2: /* create new group test_group */
            if ((msg_handle = ov_create_group(ov_handle, test_group_name, &gp)))
            {
                ov_msg_cleanup(msg_handle);
                test_group = gp.id;
            }
            if (test_group == 0) {
                test_group = ov_group_name_to_id(ov_handle, test_group_name);
            }
            rc = (msg_handle && test_group)? 0 : 1;
            break;
        case 3: /* get group list again, has new group been added? */
            rc = demo_print_group_list(ov_handle, 0);
            break;
        case 4: /* delete group test_group */
            if ((msg_handle = ov_delete_group(ov_handle, test_group)))
            {
                ov_msg_cleanup(msg_handle);
            }
            rc = msg_handle ? 0 : 1;
            break;
        case 5: /* get group list again, group should have gone */
            rc = demo_print_group_list(ov_handle, 0);
            break;
        case 6: /* create new group test_group again */
            if ((msg_handle = ov_create_group(ov_handle, test_group_name, &gp)))
            {
                ov_msg_cleanup(msg_handle);
                test_group = gp.id;
            }
            if (test_group == 0) {
                test_group = ov_group_name_to_id(ov_handle, test_group_name);
            }
             rc = msg_handle ? 0 : 1;
            break;
        case 7:   /* Show all members of all groups */
        {
            ov_group_list_param_t gp = {};
            if ((msg_handle = ov_get_group_list(ov_handle, &gp)))
            {
                ov_group_list_status_t *ge;
                for (i = 0, ge = gp.entries; (i < gp.entry_count) && (rc == 0); i++, ge++)
                {
                    rc = demo_print_member_list(ov_handle, ge->id);
                }
                ov_msg_cleanup(msg_handle);
            }
            rc = msg_handle ? 0 : 1;
            break;
        }
        case 8:   /* Add a member to test_group */

        {
            ov_member_param_t mp =
            {
                .group_id = test_group,
                .first = "Sally",                    // the first name
                .last = "Jones",                     // the last name
                .phone = "3105551212",                    // the US phone number no special characters or white space (if type SMS)
                .email = "aunt.sally@aol.com",           // the email of the person (only if the type is email)
                .type = OV_MEMBER_TYPE_EMAIL            // the contact preference type
            };
            if ((msg_handle = ov_create_member(ov_handle, &mp)))
            {
                ov_msg_cleanup(msg_handle);
            }
            rc = msg_handle ? 0 : 1;
            break;
        }
        case 9: /* Show all members of test_group; "Sally Jones" should have been added */
            rc = demo_print_member_list(ov_handle, test_group);
            break;
        case 10:   /* Delete "Sally Jones" from group test_group */
        {
            /* Scan the member list to find the member ID for Sally Jones */
            ov_member_list_t mp =
            {
                .entry_count = -1,
                .group_id = test_group
            };
            if ((msg_handle = ov_get_member_list(ov_handle, &mp)))
            {
                ov_member_status_t *me;
                for (i = 0, me = mp.entries; i < mp.entry_count; i++, me++)
                {
                    if (strcmp(me->first_name, "Sally") || strcmp(me->last_name, "Jones"))
                    {
                        /* this is not Sally, skip */
                        continue;
                    }
                    /* found it - delete it */
                    if ((msg_handle2 = ov_delete_member(ov_handle, test_group, me->id)))
                    {
                        ov_msg_cleanup(msg_handle2);
                    }
                    rc = msg_handle2 ? 0 : 1;
                    break;
                }
                if (i == mp.entry_count)
                {
                    /* member not found, error */
                    rc = 1;
                }
                ov_msg_cleanup(msg_handle);
            }
            break;
        }
        case 11: /* Show all members of test_group; "Sally Jones" should have been removed */
            rc = demo_print_member_list(ov_handle, test_group);
            break;
        case 12:   /* Send 3 messages to test_group */
        {
            ov_create_message_t cm =
            {
                .group_id = test_group,
                .notify_member = 1,
            };
            char msg_buf[64];
            for (i = 0; (i < 3) && (rc == 0); i++)
            {
                sprintf(msg_buf, "OpenVersa API Test Message %d", i + 1);
                if ((msg_handle = ov_create_message(ov_handle, &cm, msg_buf)))
                {
                    ov_msg_cleanup(msg_handle);
                }
                rc = msg_handle ? 0 : 1;
            }
            break;
        }
        case 13: /* Retrieve the messages that were sent to test_group.
                  * Should see "OpenVersa API Test Message" in the list
                  */
            rc = demo_print_messages(ov_handle, test_group);
            break;
        case 14:   /* Remove all messages from the group test_group */
        {
            ov_get_message_t *ms;
            ov_get_message_list_t mp = { .group_id = test_group };
            if ((msg_handle = ov_get_messages(ov_handle, test_group, &mp)))
            {
                for (i = 0, ms = mp.messages; rc == 0 && (i < mp.entry_count); i++, ms++)
                {
                    if ((msg_handle2 = ov_delete_message(ov_handle, ms->id)))
                    {
                        ov_msg_cleanup(msg_handle2);
                    }
                    rc = msg_handle2 ? 0 : 1;
                }
                ov_msg_cleanup(msg_handle);
            }
            rc |= msg_handle ? 0 : 1;
            break;
        }
        case 15: /* Retrieve the messages that were sent to test_group.
                  * Should be empty
                  */
            rc = demo_print_messages(ov_handle, test_group);
            break;
        case 16: /* get file list for group test_group, should be empty */
            rc = demo_print_file_list(ov_handle, test_group);
            break;
        case 17:   /* upload file "abc.txt" group test_group */
        {
            FILE *fp = fopen("abc.txt", "w");
            if (fp)
            {
                ov_put_file_param_t pfp =
                {
                    .notify_member = 0,
                    .name = "abc.txt",
                    .title = "abc.txt"
                };
                fwrite("TEST\n", 1, sizeof("TEST\n"), fp);
                fclose(fp);

                if ((msg_handle = ov_put_file(ov_handle, test_group, &pfp)))
                {
                    ov_msg_cleanup(msg_handle);
                }
            }
            rc = msg_handle ? 0 : 1;
            break;
        }
        case 18: /* get file list for group test_group, is "abc.txt" there? */
            rc = demo_print_file_list(ov_handle, test_group);
            break;
        case 19:   /* download file "abc.txt" group test_group */
        {
            ov_get_file_param_t gfp =
            {
                .name = "abc.txt",
                .local_path = "def.txt"
            };
            /* first, delete local file, just to be sure */
            unlink(gfp.local_path);

            if ((msg_handle = ov_get_file(ov_handle, test_group, &gfp)))
            {
                ov_msg_cleanup(msg_handle);
            }
            rc = msg_handle ? 0 : 1;
            /* verify correct download */
            {
                FILE *fp1 = fopen("abc.txt", "r");
                FILE *fp2 = fopen("def.txt", "r");
                if (fp1 && fp2) {
                    char c1, c2;
                    while (((c1 = getc(fp1)) == (c2 = getc(fp2))) && (c1 != EOF)) {
                    }
                    if (c1 != EOF) {
                        printf("file downloaded different to uploaded\n");
                        rc = 1;
                    }
                    else {
                        printf("Success: downloaded file is identical to uploaded file!\n");
                    }
                }
                else {
                    printf("file does not exist\n");
                    rc = 1;
                }
                if (fp1) fclose(fp1);
                if (fp2) fclose(fp2);
            }
            break;
        }
        case 20: /* Show rules for group 'OV_TEST_ABC'. Should be empty */
            rc = demo_print_rules(ov_handle, test_group);
            break;
        case 21: /*  Add Group 'OV_Test_GHI' and 2 rules */
            if ((msg_handle = ov_create_group(ov_handle, "OV_Test_GHI", &gp)))
            {

                ov_rule_event_t re1 =
                {
                    .type = TYPE_TRIGGER,
                    .u.trigger.search_text = "trig-search-text",
                };
                ov_rule_event_t re2 =
                {
                    .type = TYPE_COUNT,
                    .u.count = { .interval = 10, .limit = 20 }
                };
                ov_rule_action_t ra =
                {
                    .type = TELL_GROUP,
                    .u.group.name = "OV_Test_GHI",
                    .include_original_message = 1,
                    .new_message = "this is a new message"
                };
                ov_rule_t r1 =
                {
                    .name = "Rule One",
                    .description = "This is a trigger rule",
                    .active = 1,
                    .direction = DIR_SEND,
                    .event = &re1,
                    .action = &ra,
                };
                ov_rule_t r2 =
                {
                    .name = "Rule Two",
                    .description = "This is a count rule",
                    .active = 1,
                    .direction = DIR_RECEIVE,
                    .event = &re2,
                    .action = &ra,
                };
                if ((msg_handle2 = ov_rule_create(ov_handle, test_group, &r1)))
                {
                    ov_msg_cleanup(msg_handle2);
                }
                if ((msg_handle2 = ov_rule_create(ov_handle, test_group, &r2)))
                {
                    ov_msg_cleanup(msg_handle2);
                }
                ov_msg_cleanup(msg_handle);
            }
            rc = msg_handle ? 0 : 1;
            break;
        case 22: /* Get rule list for Group 'OV_TEST_ABC', Are 2 rules there? */
            rc = demo_print_rules(ov_handle, test_group);
            break;
        case 23:   /* Delete Group 2 rules and 'OV_Test_GHI' */
        {
            ov_get_rule_list_t grl = {};
            if ((msg_handle = ov_get_rules(ov_handle, test_group, &grl)))
            {
                for (i = 0; i < grl.entry_count; i++)
                {
                    if ((msg_handle2 = ov_delete_rule(ov_handle, test_group, grl.rules[i].id)))
                    {
                        ov_msg_cleanup(msg_handle2);
                    }
                }
                ov_msg_cleanup(msg_handle);
            }
            if ((msg_handle = ov_delete_group(ov_handle, gp.id)))
            {
                ov_msg_cleanup(msg_handle);
            }
            rc = msg_handle ? 0 : 1;
            break;
        }
        case 24: /* Show rules for group 'OV_TEST_ABC'. Should be empty */
            rc = demo_print_rules(ov_handle, test_group);
            break;
        case 25: /* Get Description for group'OV_TEST_ABC', should be empty */
        {
            ov_group_description_param_t gdp = {
                .group_id = test_group,
            };
            if ((msg_handle = ov_get_group_description(ov_handle, &gdp))) {
                printf("group %d description = \n%s\n", test_group, gdp.description);
                ov_msg_cleanup(msg_handle);
            }
            rc = msg_handle ? 0 : 1;
            break;
        }
        case 26: /* Set Description for group'OV_TEST_ABC' to 'lkjhgfdsa' */
        {
            ov_group_description_param_t gdp = {
                .group_id = test_group,
                .description = "lkjhgfdsa"
            };
            if ((msg_handle = ov_set_group_description(ov_handle, &gdp))) {
                printf("set group %d description to \n%s\n", test_group, gdp.description);
                ov_msg_cleanup(msg_handle);
            }
            rc = msg_handle ? 0 : 1;
            break;
        }
        case 27: /* Get Description for group'OV_TEST_ABC', should be 'lkjhgfdsa' */
        {
            ov_group_description_param_t gdp = {
                .group_id = test_group,
            };
            if ((msg_handle = ov_get_group_description(ov_handle, &gdp))) {
                printf("group %d description = \n%s\n", test_group, gdp.description);
                ov_msg_cleanup(msg_handle);
            }
            rc = msg_handle ? 0 : 1;
            break;
        }

        case 28: /* Get Data store for group'OV_TEST_ABC', should be empty */
        {
            ov_group_datastore_param_t gdp = {
                .group_id = test_group,
            };
            if ((msg_handle = ov_get_group_datastore(ov_handle, &gdp))) {
                printf("group %d datastore = \n%s\n", test_group, gdp.datastore);
                ov_msg_cleanup(msg_handle);
            }
            rc = msg_handle ? 0 : 1;
            break;
        }
        case 29: /* Set Data store for group'OV_TEST_ABC' to 'lkjhgfdsa' */
        {
            ov_group_datastore_param_t gdp = {
                .group_id = test_group,
                .datastore = "lkjhgfdsa"
            };
            if ((msg_handle = ov_set_group_datastore(ov_handle, &gdp))) {
                printf("set group %d datastore to \n%s\n", test_group, gdp.datastore);
                ov_msg_cleanup(msg_handle);
            }
            rc = msg_handle ? 0 : 1;
            break;
        }
        case 30: /* Get Data store for group'OV_TEST_ABC', should be 'lkjhgfdsa' */
        {
            ov_group_datastore_param_t gdp = {
                .group_id = test_group,
            };
            if ((msg_handle = ov_get_group_datastore(ov_handle, &gdp))) {
                printf("group %d datastore = \n%s\n", test_group, gdp.datastore);
                ov_msg_cleanup(msg_handle);
            }
            rc = msg_handle ? 0 : 1;
            break;
        }
        case 31: /* delete group test_group */
            if ((msg_handle = ov_delete_group(ov_handle, test_group)))
            {
                ov_msg_cleanup(msg_handle);
            }
            rc = msg_handle ? 0 : 1;
            break;
        case 32: /* get group list again, group should have gone */
            rc = demo_print_group_list(ov_handle, 0);
            break;
        default:
            /* we're done */
            rc = 1;
            break;
        }
    }
    return 0;
}
