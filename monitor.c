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
#include "ov_api.h"

#define MSG_COUNT 20
#define MESSAGE_POLL_INTERVAL 5
#define STATS_REPORT_INTERVAL 11

#define LINUX_RESP_BUF_SIZE 8192

static void mon_send_message(ov_handle_t *ov_handle, int group_id, char *buf)
{
    if (ov_handle) {
        ov_create_message_t cm = {
            .group_id = group_id,
            .notify_member = 1,
        };
        ov_msg_handle_t *msg_handle = ov_create_message(ov_handle, &cm, buf);

        if (msg_handle) {
            ov_msg_cleanup(msg_handle);
        }
    }
}

static int mon_group_init(ov_handle_t *ov_handle, char *group_name, int *last_msg_id, char **keepalive_url)
{
	ov_msg_handle_t *msg_handle;
	int group_id = -1, i = 0;
    ov_get_group_details_t ggd = {};
    ov_group_param_t gp = { .auto_location = 1, .type = 1 };

    /* Create the group. Keep retrying until internet connection is up */
    while (!(msg_handle = ov_create_group(ov_handle, group_name, &gp))) {
        printf("waiting for connection...[%d]\n", ++i);
        sleep(5);
    }
    ov_msg_cleanup(msg_handle);

	/* Get the group id */
    msg_handle = ov_get_group_details(ov_handle, group_name, &ggd);
    if (msg_handle) {
        /* success */
        printf("get group details OK - group id = %d\n", ggd.id);
        group_id = ggd.id;
        *last_msg_id = ggd.last_msg_id;
        *keepalive_url = strdup(ggd.keepalive_url);
        ov_msg_cleanup(msg_handle);
    }
    else {
        printf("get group details failed\n");
        return -1;
    }
	return group_id;
}

int ov_main_monitor(ov_handle_t *ov_handle, int argc, char *argv[])
{
	char group_name[32] = "TPLINK-MR3040-";
	ov_msg_handle_t *msg_handle, *msg_handle2;
	int msg_poll_timer = 0;
    int entry_count_saved;
    ov_get_message_list_t gml = {
        .entry_count = MSG_COUNT,
    };
    int group_change_id = -1, group_change_id_new;
	char *keepalive_url = NULL;
    FILE *fp;

    if (argc > 1) {
        strncpy(group_name, argv[1], sizeof(group_name) - 1);
    }
    else {
        /* make a unique group name from MAC address */
        if ((fp = fopen("/sys/class/net/eth0/address", "r"))) {
            char buf[18];
            int l = fread(buf, 1, sizeof(buf), fp);
            fclose(fp);
            if (l == sizeof(buf)) {
                int i;
                char *p = group_name + strlen(group_name);
                for (i = 0; i < 6; i++) {
                    memcpy(p + (2 * i), buf + (3 * i), 2);
                }
                *(p + 12) = '\0';
            }
        }
        else {
            /* MAC not available */
            strcpy(group_name + strlen(group_name), "xxxxxx");
        }
    }

    gml.group_id = mon_group_init(ov_handle, group_name, &gml.msg_id_next, &keepalive_url);

    /* upload a file */
    system("dmesg > dmesg.log");
    {
        ov_put_file_param_t pfp = {
            .notify_member = 1,
            .name = "dmesg.log",
            .title = "dmesg.log"
        };
        ov_msg_handle_t *msg_handle = ov_put_file(ov_handle, gml.group_id, &pfp);
        if (msg_handle) {
            ov_msg_cleanup(msg_handle);
        }
    }

    while (1) {
        if (msg_poll_timer++ > MESSAGE_POLL_INTERVAL) {
            /* poll for messages */
            ov_get_message_t *ms;
            int i;
            msg_poll_timer = 0;

		    if (ov_group_keepalive(keepalive_url, &group_change_id_new) == 0) {
		        /* success */
		        if (group_change_id_new == group_change_id) {
		            /* no change, poll later */
		            goto done;
		        }
		        /* something changed, update group_change_id and query OV */
		        group_change_id = group_change_id_new;
		    }

            while ((gml.entry_count == MSG_COUNT) &&
                   ((msg_handle = ov_get_messages(ov_handle, gml.group_id, &gml)))) {
                /* messages received OK */
                printf("retrieved %d messages, next id = %d\n", gml.entry_count, gml.msg_id_next);
                for (i = 0, ms = gml.messages; i < gml.entry_count; i++, ms++) {
                    printf("Message %d[%d]\n", i, ms->id);
                    if (*ms->message == '!') {
                        /* prefix with "!" means issue a Linux command */
                        char buf[LINUX_RESP_BUF_SIZE];
                        char *ptr = buf;
                        /* first, delete the message */
                        if ((msg_handle2 = ov_delete_message(ov_handle, ms->id))) {
                            /* deleted OK */
                            ov_msg_cleanup(msg_handle2);
                        }

                        /* strip out the backslashes */
                        ov_trim(ms->message);

                        if ((fp = popen(&ms->message[1], "r"))) {
                            ptr += snprintf(ptr, LINUX_RESP_BUF_SIZE - 1, "Issued command [%s]\n", &ms->message[1]);
                            while (fgets(ptr, LINUX_RESP_BUF_SIZE - (ptr - buf) - 2, fp)) {
                                ptr += strlen(ptr);
                                if ((ptr - buf) > (LINUX_RESP_BUF_SIZE - 256)) {
                                    /* overrun */
                                    strcpy(ptr, "...[overrun]\n");
                                    break;
                                }
                            }
                            pclose(fp);
                            mon_send_message(ov_handle, gml.group_id, buf);
                        }
                    }
                }
                entry_count_saved = gml.entry_count;
                ov_msg_cleanup(msg_handle);
                gml.entry_count = entry_count_saved;
            }
        }
        gml.entry_count = MSG_COUNT;
done:
        sleep(1);
    }
	/* done */
	printf("done\n");
	return 0;
}
