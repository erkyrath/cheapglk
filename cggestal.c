#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "glk.h"
#include "cheapglk.h"

typedef struct glkfunction_struct {
    glui32 id;
    void *fnptr;
    char *name;
} glkfunction_t;

static glkfunction_t *find_function_by_id(glui32 id);
static glkfunction_t *find_function_by_name(char *name);

glui32 glk_gestalt(glui32 id, glui32 val)
{
    return glk_gestalt_ext(id, val, NULL);
}

glui32 glk_gestalt_ext(glui32 id, glui32 val, void *ptr)
{
    int ix;
    
    switch (id) {
        
        case gestalt_Version:
            return 0x00000400;
        
        case gestalt_FunctionIDToName: {
            glkfunction_t *func = find_function_by_id(val);
            if (!func)
                return FALSE;
            if (ptr) {
                *((char **)ptr) = func->name;
            }
            return TRUE;
        }
        
        case gestalt_FunctionNameToID: {
            glkfunction_t *func;
            if (!ptr)
                return 0;
            func = find_function_by_name(ptr);
            if (!func)
                return 0;
            else
                return func->id;
        }
        
        case gestalt_LineInput:
            if (val >= 32 && val < 127)
                return TRUE;
            else
                return FALSE;
                
        case gestalt_CharInput: 
            if (val >= 32 && val < 127)
                return TRUE;
            else if (val == keycode_Return)
                return TRUE;
            else
                return FALSE;
        
        case gestalt_CharOutput: 
            if (val >= 32 && val < 127) {
                if (ptr)
                    *((glui32 *)ptr) = 1;
                return gestalt_CharOutput_ExactPrint;
            }
            else {
                /* cheaply, we don't do any translation of printed
                    characters, so the output is always one character 
                    even if it's wrong. */
                if (ptr)
                    *((glui32 *)ptr) = 1;
                return gestalt_CharOutput_CannotPrint;
            }
            
        case gestalt_MouseInput: 
            return FALSE;
            
        case gestalt_Timer: 
            return FALSE;
            
        default:
            return 0;

    }
}

/* The functions in this table must be ordered by id. */
static glkfunction_t function_table[] = {
    0x0001, glk_exit, "exit",
    0x0002, glk_set_interrupt_handler, "set_interrupt_handler",
    0x0003, glk_tick, "tick",
    0x0004, glk_gestalt, "gestalt",
    0x0005, glk_gestalt_ext, "gestalt_ext",
    0x0020, glk_window_iterate, "window_iterate",
    0x0021, glk_window_get_rock, "window_get_rock",
    0x0022, glk_window_get_root, "window_get_root",
    0x0023, glk_window_open, "window_open",
    0x0024, glk_window_close, "window_close",
    0x0025, glk_window_get_size, "window_get_size",
    0x0026, glk_window_set_arrangement, "window_set_arrangement",
    0x0027, glk_window_get_arrangement, "window_get_arrangement",
    0x0028, glk_window_get_type, "window_get_type",
    0x0029, glk_window_get_parent, "window_get_parent",
    0x002A, glk_window_clear, "window_clear",
    0x002B, glk_window_move_cursor, "window_move_cursor",
    0x002C, glk_window_get_stream, "window_get_stream",
    0x002D, glk_window_set_echo_stream, "window_set_echo_stream",
    0x002E, glk_window_get_echo_stream, "window_get_echo_stream",
    0x002F, glk_set_window, "set_window",
    0x0040, glk_stream_iterate, "stream_iterate",
    0x0041, glk_stream_get_rock, "stream_get_rock",
    0x0042, glk_stream_open_file, "stream_open_file",
    0x0043, glk_stream_open_memory, "stream_open_memory",
    0x0044, glk_stream_close, "stream_close",
    0x0045, glk_stream_set_position, "stream_set_position",
    0x0046, glk_stream_get_position, "stream_get_position",
    0x0047, glk_stream_set_current, "stream_set_current",
    0x0048, glk_stream_get_current, "stream_get_current",
    0x0060, glk_fileref_create_temp, "fileref_create_temp",
    0x0061, glk_fileref_create_by_name, "fileref_create_by_name",
    0x0062, glk_fileref_create_by_prompt, "fileref_create_by_prompt",
    0x0063, glk_fileref_destroy, "fileref_destroy",
    0x0064, glk_fileref_iterate, "fileref_iterate",
    0x0065, glk_fileref_get_rock, "fileref_get_rock",
    0x0066, glk_fileref_delete_file, "fileref_delete_file",
    0x0067, glk_fileref_does_file_exist, "fileref_does_file_exist",
    0x0080, glk_put_char, "put_char",
    0x0081, glk_put_char_stream, "put_char_stream",
    0x0082, glk_put_string, "put_string",
    0x0083, glk_put_string_stream, "put_string_stream",
    0x0084, glk_put_buffer, "put_buffer",
    0x0085, glk_put_buffer_stream, "put_buffer_stream",
    0x0086, glk_set_style, "set_style",
    0x0087, glk_set_style_stream, "set_style_stream",
    0x0090, glk_get_char_stream, "get_char_stream",
    0x0091, glk_get_line_stream, "get_line_stream",
    0x0092, glk_get_buffer_stream, "get_buffer_stream",
    0x00A0, glk_char_to_lower, "char_to_lower",
    0x00A1, glk_char_to_upper, "char_to_upper",
    0x00B0, glk_stylehint_set, "stylehint_set",
    0x00B1, glk_stylehint_clear, "stylehint_clear",
    0x00B2, glk_style_distinguish, "style_distinguish",
    0x00B3, glk_style_measure, "style_measure",
    0x00C0, glk_select, "select",
    0x00C1, glk_select_poll, "select_poll",
    0x00D0, glk_request_line_event, "request_line_event",
    0x00D1, glk_cancel_line_event, "cancel_line_event",
    0x00D2, glk_request_char_event, "request_char_event",
    0x00D3, glk_cancel_char_event, "cancel_char_event",
    0x00D4, glk_request_mouse_event, "request_mouse_event",
    0x00D5, glk_cancel_mouse_event, "cancel_mouse_event",
    0x00D6, glk_request_timer_events, "request_timer_events",
};

#define NUMFUNCTIONS (sizeof(function_table) / sizeof(glkfunction_t))

static glkfunction_t *find_function_by_id(glui32 id)
{
    int top, bot, val;
    glkfunction_t *func;
    
    bot = 0;
    top = NUMFUNCTIONS;
    
    while (1) {
        val = (top+bot) / 2;
        func = &(function_table[val]);
        if (func->id == id)
            return func;
        if (bot >= top-1)
            break;
        if (func->id < id) {
            bot = val+1;
        }
        else {
            top = val;
        }
    }
    
    return NULL;
}

static int glkfunc_compare(glkfunction_t **v1, glkfunction_t **v2)
{
    return strcmp((*v1)->name, (*v2)->name);
}

static glkfunction_t *find_function_by_name(char *name)
{
    static glkfunction_t *sortedtable[NUMFUNCTIONS];
    static int built = FALSE;

    int top, bot, val, cmpval;
    glkfunction_t *func;
    
    if (!built) {
        for (val=0; val<NUMFUNCTIONS; val++) {
            sortedtable[val] = &(function_table[val]);
        }
        qsort(sortedtable, NUMFUNCTIONS, sizeof(glkfunction_t *), (void *)glkfunc_compare);
        built = TRUE;
    }
    
    bot = 0;
    top = NUMFUNCTIONS;
    
    while (1) {
        val = (top+bot) / 2;
        func = sortedtable[val];
        cmpval = strcmp(func->name, name);
        if (cmpval == 0)
            return func;
        if (bot >= top-1)
            break;
        if (cmpval < 0) {
            bot = val+1;
        }
        else {
            top = val;
        }
    }

    return NULL;
}
