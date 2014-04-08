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

void gidebug_perform_command(char *cmd)
{
    if (!gidebug_debugging_is_available()) {
#if GIDEBUG_LIBRARY_SUPPORT
        gidebug_output("The interpreter does not have a debug feature.");
#endif /* GIDEBUG_LIBRARY_SUPPORT */
        return;
    }

    debug_handler(cmd);
}

