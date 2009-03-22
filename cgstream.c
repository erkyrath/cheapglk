#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "glk.h"
#include "cheapglk.h"

/* This implements pretty much what any Glk implementation needs for 
    stream stuff. Memory streams, file streams (using stdio functions), 
    and window streams (which just print to stdout.) A fancier 
    implementation would need to change the window stream stuff, but 
    memory and file streams would stay the same. (Unless you're on a 
    wacky platform like the Mac and want to change stdio to native file 
    functions.) 
*/

static stream_t *gli_streamlist = NULL; /* linked list of all streams */
static stream_t *gli_currentstr = NULL; /* the current output stream */

stream_t *gli_new_stream(int type, int readable, int writable, 
    glui32 rock)
{
    stream_t *str = (stream_t *)malloc(sizeof(stream_t));
    if (!str)
        return NULL;
    
    str->magicnum = MAGIC_STREAM_NUM;
    str->type = type;
    str->rock = rock;
    
    str->win = NULL;
    str->file = NULL;
    str->buf = NULL;
    str->bufptr = NULL;
    str->bufend = NULL;
    str->bufeof = NULL;
    str->buflen = 0;
    
    str->readcount = 0;
    str->writecount = 0;
    str->readable = readable;
    str->writable = writable;
    
    str->next = gli_streamlist;
    gli_streamlist = str;
    
    return str;
}

void gli_delete_stream(stream_t *str)
{
    window_t *win;
    stream_t **strptr;
    
    if (str == gli_currentstr) {
        gli_currentstr = NULL;
    }
    
    win = gli_window_get();
    if (win && win->echostr == str) {
        win->echostr = NULL;
    }
    
    str->magicnum = 0;

    switch (str->type) {
        case strtype_Window:
            /* nothing necessary; the window is already being closed */
            break;
        case strtype_Memory: 
            /* nothing necessary; the data is already there */
            break;
        case strtype_File:
            /* close the FILE */
            fclose(str->file);
            str->file = NULL;
            break;
    }
    
    /* yank str from the linked list. */
    for (strptr = &(gli_streamlist); 
        *strptr; 
        strptr = &((*strptr)->next)) {
        if (*strptr == str) {
            *strptr = str->next;
            break;
        }
    }
    str->next = NULL;
    
    free(str);
}

void gli_stream_fill_result(stream_t *str, stream_result_t *result)
{
    if (!result)
        return;
    
    result->readcount = str->readcount;
    result->writecount = str->writecount;
}

void glk_stream_close(strid_t id, stream_result_t *result)
{
    stream_t *str;

    if (!id || !(str = IDToStream(id))) {
        gli_strict_warning("stream_close: invalid id.");
        return;
    }
    
    if (str->type == strtype_Window) {
        gli_strict_warning("stream_close: cannot close window stream");
        return;
    }
    
    gli_stream_fill_result(str, result);
    gli_delete_stream(str);
}

strid_t glk_stream_open_memory(void *buf, glui32 buflen, glui32 fmode, 
    glui32 rock)
{
    stream_t *str;
    
    if (fmode != filemode_Read 
        && fmode != filemode_Write 
        && fmode != filemode_ReadWrite) {
        gli_strict_warning("stream_open_memory: illegal filemode");
        return 0;
    }
    
    str = gli_new_stream(strtype_Memory, 
        (fmode != filemode_Write), 
        (fmode != filemode_Read), 
        rock);
    if (!str) {
        gli_strict_warning("stream_open_memory: unable to create stream.");
        return 0;
    }
    
    if (buf && buflen) {
        str->buf = buf;
        str->bufptr = buf;
        str->buflen = buflen;
        str->bufend = str->buf + str->buflen;
        if (fmode == filemode_Write)
            str->bufeof = buf;
        else
            str->bufeof = str->bufend;
    }
    
    return StreamToID(str);
}

strid_t glk_stream_open_file(frefid_t frefid, glui32 fmode,
    glui32 rock)
{
    char modestr[16];
    fileref_t *fref;
    stream_t *str;
    FILE *fl;
    
    if (!frefid || !(fref = IDToFileref(frefid))) {
        gli_strict_warning("stream_open_file: invalid fileref id.");
        return 0;
    }
    
    switch (fmode) {
        case filemode_Write:
            strcpy(modestr, "w");
            break;
        case filemode_Read:
            strcpy(modestr, "r");
            break;
        case filemode_ReadWrite:
            strcpy(modestr, "r+");
            break;
        case filemode_WriteAppend:
            strcpy(modestr, "a");
            break;
    }
    
    if (!fref->textmode)
        strcat(modestr, "b");
        
    fl = fopen(fref->filename, modestr);
    if (!fl) {
        gli_strict_warning("stream_open_file: unable to open file.");
        return 0;
    }

    str = gli_new_stream(strtype_File, 
        (fmode == filemode_Read || fmode == filemode_ReadWrite), 
        !(fmode == filemode_Read), 
        rock);
    if (!str) {
        gli_strict_warning("stream_open_file: unable to create stream.");
        fclose(fl);
        return 0;
    }
    
    str->file = fl;
    
    return StreamToID(str);
}

strid_t glk_stream_iterate(strid_t id, glui32 *rockptr)
{
    stream_t *str;

    if (!id) {
        if (gli_streamlist) {
            if (rockptr)
                *rockptr = gli_streamlist->rock;
            return StreamToID(gli_streamlist);
        }
        else {
            if (rockptr)
                *rockptr = 0;
            return 0;
        }
    }
    else {
        str = IDToStream(id);
        if (!str) {
            gli_strict_warning("stream_iterate: invalid id.");
            return 0;
        }
        str = str->next;
        if (str) {
            if (rockptr)
                *rockptr = str->rock;
            return StreamToID(str);
        }
        else {
            if (rockptr)
                *rockptr = 0;
            return 0;
        }
    }
}

glui32 glk_stream_get_rock(strid_t id)
{
    stream_t *str;

    if (!id || !(str = IDToStream(id))) {
        gli_strict_warning("stream_get_rock: invalid id.");
        return 0;
    }
    
    return str->rock;
}

void gli_stream_set_current(stream_t *str)
{
    gli_currentstr = str;
}

void glk_stream_set_current(strid_t id)
{
    stream_t *str;
    
    if (!id) {
        str = NULL;
    }
    else {
        str = IDToStream(id);
        if (!str) {
            gli_strict_warning("stream_set_current: invalid id.");
            return;
        }
    }

    gli_stream_set_current(str);
}

strid_t glk_stream_get_current()
{
    if (gli_currentstr)
        return StreamToID(gli_currentstr);
    else
        return 0;
}

void glk_stream_set_position(strid_t id, glsi32 pos, glui32 seekmode)
{
    stream_t *str;
    
    if (!id || !(str = IDToStream(id))) {
        gli_strict_warning("stream_set_position: invalid id");
        return;
    }

    switch (str->type) {
        case strtype_Memory: 
            if (seekmode == seekmode_Current) {
                pos = (str->bufptr - str->buf) + pos;
            }
            else if (seekmode == seekmode_End) {
                pos = (str->bufeof - str->buf) + pos;
            }
            else {
                /* pos = pos */
            }
            if (pos < 0)
                pos = 0;
            if (pos > (str->bufeof - str->buf))
                pos = (str->bufeof - str->buf);
            str->bufptr = str->buf + pos;
            break;
        case strtype_Window:
            /* do nothing; don't pass to echo stream */
            break;
        case strtype_File:
            fseek(str->file, pos, 
                ((seekmode == seekmode_Current) ? 1 :
                ((seekmode == seekmode_End) ? 2 : 0)));
            break;
    }   
}

glui32 glk_stream_get_position(strid_t id)
{
    stream_t *str;
    
    if (!id || !(str = IDToStream(id))) {
        gli_strict_warning("stream_get_position: invalid id");
        return 0;
    }

    switch (str->type) {
        case strtype_Memory: 
            return (str->bufptr - str->buf);
        case strtype_File:
            return ftell(str->file);
        case strtype_Window:
        default:
            return 0;
    }   
}

static void gli_put_char(stream_t *str, unsigned char ch)
{
    if (!str || !str->writable)
        return;

    str->writecount++;
    
    switch (str->type) {
        case strtype_Memory:
            if (str->bufptr < str->bufend) {
                *(str->bufptr) = ch;
                str->bufptr++;
                if (str->bufptr > str->bufeof)
                    str->bufeof = str->bufptr;
            }
            break;
        case strtype_Window:
            if (str->win->line_request) {
                gli_strict_warning("put_char: window has pending line request");
                break;
            }
            /* If you're going to convert Latin-1 to a different 
                character set, this is (a) place to do it. Only on the 
                putc(); not on the gli_put_char to echostr. */
            putc(ch, stdout);
            if (str->win->echostr)
                gli_put_char(str->win->echostr, ch);
            break;
        case strtype_File:
            putc(ch, str->file);
            break;
    }
}

static void gli_put_buffer(stream_t *str, char *buf, glui32 len)
{
    char *cx;
    
    if (!str || !str->writable)
        return;

    str->writecount += len;
    
    switch (str->type) {
        case strtype_Memory:
            if (str->bufptr >= str->bufend) {
                len = 0;
            }
            else {
                if (str->bufptr + len > str->bufend) {
                    glui32 lx;
                    lx = (str->bufptr + len) - str->bufend;
                    if (lx < len)
                        len -= lx;
                    else
                        len = 0;
                }
            }
            if (len) {
                memcpy(str->bufptr, buf, len);
                str->bufptr += len;
                if (str->bufptr > str->bufeof)
                    str->bufeof = str->bufptr;
            }
            break;
        case strtype_Window:
            if (str->win->line_request) {
                gli_strict_warning("put_buffer: window has pending line request");
                break;
            }
            /* If you're going to convert Latin-1 to a different 
                character set, this is (a) place to do it. Only on the 
                fwrite(); not on the gli_put_buffer to echostr. */
            fwrite((unsigned char *)buf, 1, len, stdout);
            if (str->win->echostr)
                gli_put_buffer(str->win->echostr, buf, len);
            break;
        case strtype_File:
            fwrite(buf, 1, len, str->file);
            break;
    }
}

void gli_stream_echo_line(stream_t *str, char *buf, glui32 len)
{
    /* This is only used to echo line input to an echo stream. See
        glk_select(). */
    gli_put_buffer(str, buf, len);
    gli_put_char(str, '\n');
}

static glui32 gli_get_char(stream_t *str)
{
    if (!str || !str->readable)
        return -1;
    
    switch (str->type) {
        case strtype_Memory:
            if (str->bufptr < str->bufend) {
                unsigned char ch;
                ch = *(str->bufptr);
                str->bufptr++;
                str->readcount++;
                return ch;
            }
            else {
                return -1;
            }
        case strtype_File: {
            int res;
            res = getc(str->file);
            if (res != -1) {
                str->readcount++;
                return (glui32)res;
            }
            else {
                return -1;
            }
            }
        case strtype_Window:
        default:
            return -1;
    }
}

static glui32 gli_get_buffer(stream_t *str, char *buf, glui32 len)
{
    if (!str || !str->readable)
        return 0;
    
    switch (str->type) {
        case strtype_Memory:
            if (str->bufptr >= str->bufend) {
                len = 0;
            }
            else {
                if (str->bufptr + len > str->bufend) {
                    glui32 lx;
                    lx = (str->bufptr + len) - str->bufend;
                    if (lx < len)
                        len -= lx;
                    else
                        len = 0;
                }
            }
            if (len) {
                memcpy(buf, str->bufptr, len);
                str->bufptr += len;
                if (str->bufptr > str->bufeof)
                    str->bufeof = str->bufptr;
            }
            str->readcount += len;
            return len;
        case strtype_File: {
            glui32 res;
            res = fread(buf, 1, len, str->file);
            str->readcount += res;
            return res;
            }
        case strtype_Window:
        default:
            return 0;
    }
}

static glui32 gli_get_line(stream_t *str, char *buf, glui32 len)
{
    glui32 lx;
    int gotnewline;

    if (!str || !str->readable)
        return 0;
    
    switch (str->type) {
        case strtype_Memory:
            if (len == 0)
                return 0;
            len -= 1; /* for the terminal null */
            if (str->bufptr >= str->bufend) {
                len = 0;
            }
            else {
                if (str->bufptr + len > str->bufend) {
                    lx = (str->bufptr + len) - str->bufend;
                    if (lx < len)
                        len -= lx;
                    else
                        len = 0;
                }
            }
            gotnewline = FALSE;
            for (lx=0; lx<len && !gotnewline; lx++) {
                buf[lx] = str->bufptr[lx];
                gotnewline = (buf[lx] == '\n');
            }
            buf[lx] = '\0';
            str->bufptr += lx;
            str->readcount += lx;
            return lx;
        case strtype_File: {
            char *res;
            res = fgets(buf, len, str->file);
            if (!res)
                return 0;
            else
                return strlen(buf);
            }
        case strtype_Window:
        default:
            return 0;
    }
}

void glk_put_char(unsigned char ch)
{
    gli_put_char(gli_currentstr, ch);
}

void glk_put_char_stream(strid_t id, unsigned char ch)
{
    stream_t *str;
    if (!id || !(str = IDToStream(id))) {
        gli_strict_warning("put_char_stream: invalid id");
        return;
    }
    gli_put_char(str, ch);
}

void glk_put_string(char *s)
{
    gli_put_buffer(gli_currentstr, s, strlen(s));
}

void glk_put_string_stream(strid_t id, char *s)
{
    stream_t *str;
    if (!id || !(str = IDToStream(id))) {
        gli_strict_warning("put_string_stream: invalid id");
        return;
    }
    gli_put_buffer(str, s, strlen(s));
}

void glk_put_buffer(char *buf, glui32 len)
{
    gli_put_buffer(gli_currentstr, buf, len);
}

void glk_put_buffer_stream(strid_t id, char *buf, glui32 len)
{
    stream_t *str;
    if (!id || !(str = IDToStream(id))) {
        gli_strict_warning("put_string_stream: invalid id");
        return;
    }
    gli_put_buffer(str, buf, len);
}

void glk_set_style(glui32 val)
{
    /* This cheap library doesn't handle styles */
}

void glk_set_style_stream(strid_t id, glui32 val)
{
    stream_t *str;
    if (!id || !(str = IDToStream(id))) {
        gli_strict_warning("set_style_stream: invalid id");
        return;
    }
    /* This cheap library doesn't handle styles */
}

glui32 glk_get_char_stream(strid_t id)
{
    stream_t *str;
    if (!id || !(str = IDToStream(id))) {
        gli_strict_warning("get_char_stream: invalid id");
        return -1;
    }
    return gli_get_char(str);
}

glui32 glk_get_line_stream(strid_t id, char *buf, glui32 len)
{
    stream_t *str;
    if (!id || !(str = IDToStream(id))) {
        gli_strict_warning("get_line_stream: invalid id");
        return -1;
    }
    return gli_get_line(str, buf, len);
}

glui32 glk_get_buffer_stream(strid_t id, char *buf, glui32 len)
{
    stream_t *str;
    if (!id || !(str = IDToStream(id))) {
        gli_strict_warning("get_buffer_stream: invalid id");
        return -1;
    }
    return gli_get_buffer(str, buf, len);
}

