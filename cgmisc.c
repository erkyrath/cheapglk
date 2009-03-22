#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "glk.h"
#include "cheapglk.h"

static unsigned char char_tolower_table[256];
static unsigned char char_toupper_table[256];

void gli_initialize_misc()
{
    int ix;
    int res;
    
    /* Initialize the to-uppercase and to-lowercase tables. These should
        *not* be localized to a platform-native character set! They are
        intended to work on Latin-1 data, and the code below correctly
        sets up the tables for that character set. */
    
    for (ix=0; ix<256; ix++) {
        char_toupper_table[ix] = ix;
        char_tolower_table[ix] = ix;
    }
    for (ix=0; ix<256; ix++) {
        if (ix >= 'A' && ix <= 'Z') {
            res = ix + ('a' - 'A');
        }
        else if (ix >= 0xC0 && ix <= 0xDE && ix != 0xD7) {
            res = ix + 0x20;
        }
        else {
            res = 0;
        }
        if (res) {
            char_tolower_table[ix] = res;
            char_toupper_table[res] = ix;
        }
    }

}

void glk_exit()
{
    exit(0);
}

void glk_set_interrupt_handler(void (*func)(void))
{
    /* This cheap library doesn't understand interrupts. */
}

unsigned char glk_char_to_lower(unsigned char ch)
{
    return char_tolower_table[ch];
}

unsigned char glk_char_to_upper(unsigned char ch)
{
    return char_toupper_table[ch];
}

void glk_select(event_t *event)
{
    window_t *win = gli_window_get();
    
    gli_event_clearevent(event);
    
    if (!win || !(win->char_request || win->line_request)) {
        /* No input requests. This is legal, but a pity, because the
            correct behavior is to wait forever. Bye bye. */
        while (1) {
            getchar();
        }
    }
    
    if (win->char_request) {
        char buf[256];
        glui32 kval;
        
        /* How cheap are we? We don't want to fiddle with line 
            buffering, so we just accept an entire line (terminated by 
            return) and use the first key. Remember that return has to 
            be turned into a special keycode (and so would other keys,
            if we could recognize them.) */
 
        fgets(buf, 255, stdin);
        kval = buf[0];
        if (kval == '\r' || kval == '\n')
            kval = keycode_Return;
        
        win->char_request = FALSE;
        event->type = evtype_CharInput;
        event->win = WindowToID(win);
        event->val1 = kval;
        
    }
    else {
        char buf[256];
        int val;
        fgets(buf, 255, stdin);
        val = strlen(buf);
        if (val && (buf[val-1] == '\n' || buf[val-1] == '\r'))
            val--;
        
        if (val > win->linebuflen)
            val = win->linebuflen;
        memcpy(win->linebuf, buf, val);
        if (win->echostr) {
            gli_stream_echo_line(win->echostr, buf, val);
        }
        
        win->line_request = FALSE;
        event->type = evtype_LineInput;
        event->win = WindowToID(win);
        event->val1 = val;
    }
}

void glk_select_poll(event_t *event)
{
    gli_event_clearevent(event);
    
    /* This only checks for timer events at the moment, and we don't
        support any, so I guess this is a pretty simple function. */
}

void glk_tick()
{
    /* Do nothing. */
}

void glk_request_timer_events(glui32 millisecs)
{
    /* Don't make me laugh. */
}
