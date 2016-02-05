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

/* Private CURL Access Functions */

#ifndef __OV_CURL__
#define __OV_CURL__

void *ov_curl_msg_init(ov_handle_t *ov_handle, const char *command,
                       void (*json_cleanup)(void*), void *data);
void ov_curl_msg_cleanup(ov_handle_t *ov_handle);

int ov_curl_set_int(void *ov_msg, const char *name, int val);
int ov_curl_set_string(void *ov_msg, const char *name, char *val);
int ov_curl_json(void *ov_msg_handle, void (*ov_json_parse_cb)(void *,
                 jsmntok_t *, char *, void*), void*);
int ov_curl_set_filedata(void *ov_msg_handle, const char *name, char *path);
int ov_curl_set_get_param(ov_handle_t *ov_handle, const char *name, char *path);
int ov_curl_set_get_param_int(void *ov_msg_handle, const char *name, int val);
int ov_curl_download(ov_handle_t *handle, char *local_path, char *url);
char *ov_curl_get(char *url);

#endif
