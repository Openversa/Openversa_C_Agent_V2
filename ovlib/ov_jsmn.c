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
#include "ov_jsmn.h"

char *jsmn_get_string(jsmntok_t *t, char *data, const char *token)
{
    char token_buf[64];
    sprintf(token_buf, "{%s", token);
    t = jsmn_tok(t, data, token_buf);
    return t ? strdup(&data[t->start]) : NULL;
}

int jsmn_get_int(jsmntok_t *t, char *data, const char *token)
{
    char token_buf[64];
    sprintf(token_buf, "{%s", token);
    t = jsmn_tok(t, data, token_buf);
    return t ? atoi(&data[t->start]) : -1;
}

jsmntok_t *jsmn_skip(jsmntok_t *t)
{
    int next_start = (t++)->end;

    while (t->end) {
        if (t->start > next_start) {
            return t;
        }
        t++;
    }
    return NULL;
}

jsmntok_t *jsmn_tok(jsmntok_t *t, char *data, char *keyword)
{
    char *ptr = keyword;
    jsmntok_t *t_obj = NULL;

    while (*ptr && t->end) {
        switch (t->type) {
        case JSMN_PRIMITIVE:
            return t->end ? t : NULL;
        case JSMN_OBJECT:
            if (*ptr == '{') {
                t_obj = t;
                ptr++;
            }
            break;
        case JSMN_ARRAY:
            if (*ptr == '[') {
                ptr++;
            }
            break;
        case JSMN_STRING:
            if (t_obj) {
                /* we're scanning for an object */
                char *p_ = strpbrk(ptr, "{[");
                int len = p_ ? (p_ - ptr) : strlen(ptr);
                if (len == (t->end - t->start) &&
                        !memcmp(&data[t->start], ptr, len)) {
                    /* this matches... */
                    ptr += (t->end - t->start);
                    t_obj = NULL;
                } else {
                    /* it does not match..skip */
                    if ((t = jsmn_skip(t + 1))) {
                        continue;
                    } else {
                        return NULL;
                    }
                }
            }
            break;
        }
        if (!(*ptr)) {
            break;
        }
        t++;
    }
    return t->end ? t + 1 : NULL;
}
