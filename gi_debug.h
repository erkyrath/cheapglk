#ifndef _GI_DEBUG_H
#define _GI_DEBUG_H

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
