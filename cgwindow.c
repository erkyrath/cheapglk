#include <stdio.h>
#include <stdlib.h>
#include "glk.h"
#include "cheapglk.h"

/* Since we're not using any kind of cursor movement or terminal 
    emulation, we're dreadfully limited in what kind of windows we
    support. In fact, we can only support one window at a time,
    and that must be a wintype_TextBuffer. Printing to this window
    simply means printing to stdout, and reading from it means
    reading from stdin. (The input code is in glk_select(), and
    the output is in glk_put_char() etc.) */

static window_t *mainwin = NULL;
static winid_t mainwin_id = 0;

window_t *gli_new_window(glui32 rock)
{
    window_t *win = (window_t *)malloc(sizeof(window_t));
    if (!win)
        return NULL;
    
    win->magicnum = MAGIC_WINDOW_NUM;
    win->rock = rock;
    
    win->str = gli_new_stream(strtype_Window, FALSE, TRUE, 0);
    win->str->win = win;
    win->echostr = NULL;
    
    win->line_request = FALSE;
    win->char_request = FALSE;
    win->linebuf = NULL;
    win->linebuflen = 0;
    
    return win;
}

void gli_delete_window(window_t *win)
{
    win->magicnum = 0;
    
    /* Close window's stream. */
    gli_delete_stream(mainwin->str);
    mainwin->str = NULL;

    /* The window doesn't own its echostr; closing the window doesn't close
        the echostr. */
    win->echostr = NULL;
    
    free(win);
}

winid_t glk_window_open(winid_t split, glui32 method, glui32 size, 
    glui32 wintype, glui32 rock)
{
    window_t *win;
    
    if (mainwin || split) {
        /* This cheap library only allows you to open a window if there
            aren't any other windows. But it's legal for the program to
            ask for multiple windows. So we don't print a warning; we just
            return 0. */
        return 0;
    }
    
    if (wintype != wintype_TextBuffer) {
        /* This cheap library only allows you to open text buffer windows. 
            Again, don't print a warning. */
        return 0;
    }
    
    win = gli_new_window(rock);
    if (!win) {
        gli_strict_warning("window_open: unable to create window.");
        return 0;
    }
    
    mainwin = win;
    mainwin_id = WindowToID(mainwin);
    return mainwin_id;
}

void glk_window_close(winid_t id, stream_result_t *result)
{
    if (!id || id != mainwin_id) {
        gli_strict_warning("window_close: invalid id.");
        return;
    }
    
    gli_stream_fill_result(mainwin->str, result);
    
    gli_delete_window(mainwin);
    mainwin = NULL;
    mainwin_id = 0;
}

window_t *gli_window_get()
{
    return mainwin;
}

winid_t glk_window_get_root()
{
    /* If there's a window, it's the root window. */
    if (mainwin)
        return mainwin_id;
    else
        return 0;
}

winid_t glk_window_iterate(winid_t id, glui32 *rockptr)
{
    /* Iteration is really simple when there can only be one window. */
    
    if (!id) {
        /* They're asking for the first window. Return the main window 
            if it exists, or 0 if there is none. */
        if (!mainwin) {
            if (rockptr)
                *rockptr = 0;
            return 0;
        }
        
        if (rockptr)
            *rockptr = mainwin->rock;
        return mainwin_id;
    }
    else if (id == mainwin_id) {
        /* They're asking for the next window. There is none. */
        if (rockptr)
            *rockptr = 0;
        return 0;
    }
    else {
        gli_strict_warning("window_iterate: invalid id.");
        return 0;
    }
}

glui32 glk_window_get_rock(winid_t id)
{
    if (!id || id != mainwin_id) {
        gli_strict_warning("window_get_rock: invalid id.");
        return 0;
    }
    
    return mainwin->rock;
}

glui32 glk_window_get_type(winid_t id)
{
    if (!id || id != mainwin_id) {
        gli_strict_warning("window_get_type: invalid id.");
        return 0;
    }
    
    return wintype_TextBuffer;
}

winid_t glk_window_get_parent(winid_t id)
{
    if (!id || id != mainwin_id) {
        gli_strict_warning("window_get_parent: invalid id.");
        return 0;
    }
    
    return 0;
}

strid_t glk_window_get_stream(winid_t id)
{
    stream_t *str;
    
    if (!id || id != mainwin_id) {
        gli_strict_warning("window_get_stream: invalid id.");
        return 0;
    }
    
    str = mainwin->str;
    
    return StreamToID(str);
}

void glk_window_set_echo_stream(winid_t id, strid_t strid)
{
    stream_t *str;

    if (!id || id != mainwin_id) {
        gli_strict_warning("window_set_echo_stream: invalid window id.");
        return;
    }

    if (!strid) {
        str = NULL;
    }
    else {
        str = IDToStream(strid);
        if (!str) {
            gli_strict_warning("window_set_echo_stream: invalid stream id.");
            return;
        }
    }
    
    mainwin->echostr = str;
}

strid_t glk_window_get_echo_stream(winid_t id)
{
    stream_t *str;
    
    if (!id || id != mainwin_id) {
        gli_strict_warning("window_get_echo_stream: invalid id.");
        return 0;
    }
    
    str = mainwin->echostr;
    
    if (str)
        return StreamToID(str);
    else
        return 0;
}

void glk_set_window(winid_t id)
{
    if (!id) {
        gli_stream_set_current(NULL);
    }
    else {
        if (id != mainwin_id) {
            gli_strict_warning("set_window: invalid id.");
            return;
        }
        gli_stream_set_current(mainwin->str);
    }
}

void glk_request_char_event(winid_t id)
{
    if (!id || id != mainwin_id) {
        gli_strict_warning("request_char_event: invalid id");
        return;
    }
    
    if (mainwin->char_request || mainwin->line_request) {
        gli_strict_warning("request_char_event: window already has keyboard request");
        return;
    }
    
    mainwin->char_request = TRUE;
}

void glk_request_line_event(winid_t id, void *buf, glui32 maxlen, glui32 initlen)
{
    if (!id || id != mainwin_id) {
        gli_strict_warning("request_line_event: invalid id");
        return;
    }
    
    if (mainwin->char_request || mainwin->line_request) {
        gli_strict_warning("request_line_event: window already has keyboard request");
        return;
    }
    
    mainwin->line_request = TRUE;
    mainwin->linebuf = buf;
    mainwin->linebuflen = maxlen;
}

void glk_request_mouse_event(winid_t id)
{
    if (!id || id != mainwin_id) {
        gli_strict_warning("request_mouse_event: invalid id");
        return;
    }
    /* Yeah, right */
    return;
}

void glk_cancel_char_event(winid_t id)
{
    if (!id || id != mainwin_id) {
        gli_strict_warning("cancel_char_event: invalid id");
        return;
    }
    
    mainwin->char_request = FALSE;
}

void glk_cancel_line_event(winid_t id, event_t *ev)
{
    event_t dummyev;
    
    if (!ev) {
        ev = &dummyev;
    }

    gli_event_clearevent(ev);
    
    if (!id || id != mainwin_id) {
        gli_strict_warning("cancel_line_event: invalid id");
        return;
    }
    
    if (mainwin->line_request) {
        mainwin->line_request = FALSE;
        mainwin->linebuf = NULL;
        mainwin->linebuflen = 0;
        
        /* Since there's only one window and no arrangement events,
            once a glk_select() starts, it can only end with actual
            line or character input. But it's possible that the
            program will set a line input request and then immediately
            cancel it. In that case, no input has occurred, so we
            set val1 to zero. */
        
        ev->type = evtype_LineInput;
        ev->val1 = 0;
        ev->val2 = 0;
        ev->win = mainwin_id;
    }
}

void glk_cancel_mouse_event(winid_t id)
{
    if (!id || id != mainwin_id) {
        gli_strict_warning("cancel_mouse_event: invalid id");
        return;
    }
    /* Yeah, right */
    return;
}

void glk_window_clear(winid_t id)
{
    int ix;
    
    if (!id || id != mainwin_id) {
        gli_strict_warning("window_clear: invalid id.");
        return;
    }
    
    if (mainwin->line_request) {
        gli_strict_warning("window_clear: window has pending line request");
        return;
    }

    for (ix=0; ix<screenheight; ix++) {
        putc('\n', stdout);
    }
}

void glk_window_move_cursor(winid_t id, glui32 xpos, glui32 ypos)
{
    if (!id || id != mainwin_id) {
        gli_strict_warning("window_move_cursor: invalid id.");
        return;
    }
    
    gli_strict_warning("window_move_cursor: cannot move cursor in a TextBuffer window.");
}

void glk_window_get_size(winid_t id, glui32 *widthptr, 
    glui32 *heightptr)
{
    if (!id || id != mainwin_id) {
        gli_strict_warning("window_get_size: invalid id.");
        return;
    }
    
    if (widthptr)
        *widthptr = screenwidth;
    if (heightptr)
        *heightptr = screenheight;
}

void glk_window_get_arrangement(winid_t id, glui32 *methodptr,
    glui32 *sizeptr, winid_t *keywinptr)
{
    if (!id || id != mainwin_id) {
        gli_strict_warning("window_get_arrangement: invalid id.");
        return;
    }
    
    gli_strict_warning("window_get_arrangement: not a Pair window.");
}

void glk_window_set_arrangement(winid_t id, glui32 method,
    glui32 size, winid_t keywin)
{
    if (!id || id != mainwin_id) {
        gli_strict_warning("window_set_arrangement: invalid id.");
        return;
    }
    
    gli_strict_warning("window_set_arrangement: not a Pair window.");
}
