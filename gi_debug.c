#include "glk.h"
#include "gi_debug.h"

static int debug_active = 0;

void gidebug_debugging_available()
{
    debug_active = 1;
}

