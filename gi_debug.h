#ifndef _GI_DEBUG_H
#define _GI_DEBUG_H

/* gi_debug.h: Debug feature layer for Glk API.
    gi_debug version 0.9.0
    Designed by Andrew Plotkin <erkyrath@eblong.com>
    http://eblong.com/zarf/glk/

    This file is copyright 2014 by Andrew Plotkin. You may copy,
    distribute, and incorporate it into your own programs, by any means
    and under any conditions, as long as you do not modify it. You may
    also modify this file, incorporate it into your own programs,
    and distribute the modified version, as long as you retain a notice
    in your program or documentation which mentions my name and the URL
    shown above.

    ------------------------------------------------

    The debug module allows a Glk library to send out-of-band debug
    commands to the game program it's linked to. (Most familiarly this
    is a Glulx interpreter, but it doesn't have to be.) The program
    returns debug output to the library, which can then display it.

    As with all UI decision, the interface of the debug feature is
    left up to the Glk library. Abstractly, we imagine a "debug
    console" window with its own input line and scrolling text output.

    The debug feature is cooperative: both the library and the game
    must support it for debug commands to work. This requires a dance
    of #ifdefs to make sure that everything builds in all
    configurations.

    Library side: If the library supports debugging, the
    GIDEBUG_LIBRARY_SUPPORT #define in this header (gi_debug.h) will
    be present (not commented out). By doing this, the library
    declares that it offers the functions gidebug_output() and
    gidebug_pause().

    Older Glk libraries do not include this header at all. Therefore,
    a game (interpreter) should have its own configuration option.
    For example, in Glulxe, you define VM_DEBUGGER when compiling with
    a library that has this header and GIDEBUG_LIBRARY_SUPPORT defined.
    When building with an older library (or a library which comments
    out the GIDEBUG_LIBRARY_SUPPORT line), you don't define VM_DEBUGGER,
    and then the interpreter does not attempt to call debug APIs.

    Game (interpreter) side: If the interpreter supports debug commands,
    it should call gidebug_debugging_available() in its startup code.
    (See unixstrt.c in the Glulxe source.) If it does not do this, the
    library knows that debug commands cannot be handled; it should
    disable or hide the "debug console" UI.
*/


/* ###debug Uncomment if the library supports a UI for debug commands. */
#define GIDEBUG_LIBRARY_SUPPORT (1)

typedef enum gidebug_cycle_enum {
    gidebug_cycle_Start        = 1,
    gidebug_cycle_End          = 2,
    gidebug_cycle_InputWait    = 3,
    gidebug_cycle_InputAccept  = 4,
    gidebug_cycle_DebugPause   = 5,
    gidebug_cycle_DebugUnpause = 6,
} gidebug_cycle;

typedef int (*gidebug_cmd_handler)(char *text);
typedef void (*gidebug_cycle_handler)(int cycle);

/* The gidebug-layer functions are always available (assuming this header
   exists!) The game should have a compile-time option (e.g. VM_DEBUGGER)
   so as not to rely on this header. */

/* ### Game calls this if it has a debugging mode. (Library controls
   whether it's used.) */
extern void gidebug_debugging_available(gidebug_cmd_handler cmdhandler, gidebug_cycle_handler cyclehandler);

/* ### Library calls this to check whether the game has a debugging mode.
   (For greying out a menu option?) */
extern int gidebug_debugging_is_available(void);

/* ### Library calls this when the user enters a debug command.
   Game should return 1 if the library should unblock and continue (after
   a pause). */
extern int gidebug_perform_command(char *cmd);

/* ### Library calls this when the game starts, stops, waits for input,
   or receives input. */
extern void gidebug_announce_cycle(gidebug_cycle cycle);

#if GIDEBUG_LIBRARY_SUPPORT

/* These functions must be implemented in the library. (If the library
   has declared debug support.) */

/* Send a line of text to the debug console. The text will be a single line
   (no newlines), and UTF-8 if necessary. */
extern void gidebug_output(char *text);

/* Block and wait for debug commands. The library will accept debug commands
   until gidebug_perform_command() returns nonzero. */
extern void gidebug_pause(void);

#endif /* GIDEBUG_LIBRARY_SUPPORT */

#endif /* _GI_DEBUG_H */
