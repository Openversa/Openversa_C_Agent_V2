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

#ifndef __OV_JSMN_H__
#define __OV_JSMN_H__
#include "jsmn.h"

jsmntok_t *jsmn_tok(jsmntok_t *t, char *data, char *keyword);
jsmntok_t *jsmn_skip(jsmntok_t *t);
char *jsmn_get_string(jsmntok_t*, char*, const char*);
int jsmn_get_int(jsmntok_t*, char*, const char*);

#endif
