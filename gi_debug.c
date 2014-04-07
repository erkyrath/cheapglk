#include "glk.h"
#include "gi_debug.h"

#ifndef NULL
#define NULL 0
#endif

static gidebug_cmd_handler debug_handler = NULL;

void gidebug_debugging_available(gidebug_cmd_handler handler)
{
    if (!handler)
        return;

    debug_handler = handler;
}

int gidebug_debugging_is_available()
{
    return (debug_handler != NULL);
}
