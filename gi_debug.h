#ifndef _GI_DEBUG_H
#define _GI_DEBUG_H

/* ###debug Uncomment if the library supports a UI for debug commands. */
#define GIDEBUG_LIBRARY_SUPPORT (1)

/* ### Game calls this if it has a debugging mode. (Library controls
   whether it's used.) */
extern void gidebug_debugging_available(void);

#if GIDEBUG_LIBRARY_SUPPORT

/* These functions must be implemented in the library. (If the library
   has declared debug support.) */

/* Send a line of text to the debug console. The text will be a single line
   (no newlines), and UTF-8 if necessary. */
extern void gidebug_output(char *text);

/* Ask the user for a line of debug input. Returns the number of characters
   entered. */
/* ### Is this what we want? What happens when the VM hits a breakpoint? */
extern int gidebug_input(char *buffer, int len);

#endif /* GIDEBUG_LIBRARY_SUPPORT */

#endif /* _GI_DEBUG_H */
