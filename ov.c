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
#include <sys/stat.h>
#include "ov_api.h"

//#error "Define your credentials here!"
#define OPENVERSA_APP_ID "mva_xxxxxxxxxxxxxxxxxxxx"
#define OPENVERSA_APP_SECRET "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define OPENVERSA_URL "https://ovapi.openversa.com/v2/xxxxxxxxxxx/index.php"
#define OPENVERSA_USER_NAME "xxxxxxxxxxxxxxxxxxxxxxxxxxxx@xxxxxxxxx.com"

extern int ov_main_test(ov_handle_t*, int argc, char *argv[]);
extern int ov_main_monitor(ov_handle_t*, int argc, char *argv[]);

typedef struct {
    const char *name;
    int (*main_proc)(ov_handle_t*, int, char*[]);
    int arg_min;
    const char *arg_description;
} ov_dispatcher_t;

static int ov_main_upload(ov_handle_t *ov_handle, int argc, char *argv[])
{
    int group_id = ov_group_name_to_id(ov_handle, argv[1]);

    if (group_id) {
        ov_put_file_param_t pfp = {
            .notify_member = 1,
            .name = argv[2],
            .title = argv[2]
        };
        ov_msg_handle_t *msg_handle = ov_put_file(ov_handle, group_id, &pfp);
        if (msg_handle) {
            ov_msg_cleanup(msg_handle);
        } else {
            printf("Error: File upload error\n");
        }
        return msg_handle ? 0 : 1;
    }
    printf("Error: Group %s not found\n", argv[1]);
    return 1;
}

/* file download */
static int ov_main_download(ov_handle_t *ov_handle, int argc, char *argv[])
{
    int group_id = ov_group_name_to_id(ov_handle, argv[1]);

    if (group_id) {
        ov_msg_handle_t *msg_handle;
        ov_get_file_param_t gfp = {
            .name = argv[2],
            .local_path = argv[3]
        };
        if ((msg_handle = ov_get_file(ov_handle, group_id, &gfp))) {
            ov_msg_cleanup(msg_handle);
        } else {
            printf("Error: File download error\n");
        }
        return msg_handle ? 0 : 1;
    }
    printf("Error: Group %s not found\n", argv[1]);
    return 1;
}

/* print out group list */
static int ov_main_group(ov_handle_t *ov_handle, int argc, char *argv[])
{
    ov_group_list_param_t gp = {
        .entry_count = -1,
        .do_badges = 0
    };
    ov_msg_handle_t *msg_handle = ov_get_group_list(ov_handle, &gp);

    if (msg_handle) {
        int i;
        ov_group_list_status_t *ge;
        for (i = 0, ge = gp.entries; i < gp.entry_count; i++, ge++) {
            printf("%s\n", ge->name);
        }
        ov_msg_cleanup(msg_handle);
    } else {
        printf("Error: Get OpenVersa group list failed\n");
    }
    return msg_handle ? 0 : 1;
}

static int ov_main_files(ov_handle_t *ov_handle, int argc, char *argv[])
{
    int group_id = ov_group_name_to_id(ov_handle, argv[1]);
    if (group_id) {
        ov_file_list_param_t flp = {};
        ov_msg_handle_t *msg_handle = ov_get_file_list(ov_handle, group_id, &flp);
        if (msg_handle) {
            int i;
            ov_file_list_status_t *fs;
            for (i = 0, fs = flp.entries; i < flp.entry_count; i++, fs++) {
                printf("%s\n", fs->name);
            }
            ov_msg_cleanup(msg_handle);
        } else {
            printf("OpenVersa file list get failed for group %s\n", argv[1]);
        }
        return msg_handle ? 0 : 1;
    }
    printf("Error: Group %s not found\n", argv[1]);
    return 1;
}

static ov_dispatcher_t main_dispatcher[] = {
    { "upload",     ov_main_upload,     3, "<group name> <local path> <ov filename>" },
    { "download",   ov_main_download,   3, "<group name> <ov filename> <local path>" },
    { "monitor",    ov_main_monitor,    1, "<group name>" },
    { "test",       ov_main_test,       0, "" },
    { "group",      ov_main_group,      0, "" },
    { "file",      ov_main_files,      1, "<group name>" },
    { NULL },
};

static int ov_usage(void)
{
    ov_dispatcher_t *md;
    printf("Usage:\n");
    for (md = main_dispatcher; md->name; md++) {
        printf("ov %s %s\n", md->name, md->arg_description);
    }
    return 1;
}

int main(int argc, char *argv[])
{
    ov_dispatcher_t *md;

    if (argc < 2) {
        return ov_usage();
    }

    for (md = main_dispatcher; md->name; md++) {
        if (!strcmp(argv[1], md->name)) {
            int ret;
            ov_handle_t *ov_handle;
            ov_param_t ovp = {
                .debug = 0,
                .get_stats = 1
            };
            if ((argc - 1) < md->arg_min) {
                return ov_usage();
            } else if (!((ov_handle = ov_init(OPENVERSA_APP_ID, OPENVERSA_APP_SECRET,
                                              OPENVERSA_USER_NAME, OPENVERSA_URL, &ovp)))) {
                printf("OpenVersa initialization failed\n");
                return 1;
            }
            ret = md->main_proc(ov_handle, argc - 1, argv + 1);
            ov_shutdown(ov_handle);
            return ret;
        }
    }
    return ov_usage();
}
