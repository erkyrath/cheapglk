#ifndef _GI_DEBUG_H
#define _GI_DEBUG_H

/* ###debug Uncomment if the library supports a UI for debug commands. */
#define GIDEBUG_LIBRARY_SUPPORT (1)

/* ### Game calls this if it has a debugging mode. (Library controls
   whether it's used.) */
extern void gidebug_debugging_available(void);

#endif /* _GI_DEBUG_H */
