#ifndef CHEAPGLK_H
#define CHEAPGLK_H

/* cheapglk.h: Private header file for Cheapass Implementation of the 
        Glk API.
    CheapGlk Library: version 0.5.
    Glk API which this implements: version 0.4.
    Designed by Andrew Plotkin <erkyrath@netcom.com>
    http://www.edoc.com/zarf/glk/index.html
*/

#define LIBRARY_VERSION "0.5"

/* First, we define our own TRUE and FALSE and NULL, because ANSI
    is a strange world. */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

/* Now, some macros to convert object IDs to internal object pointers.
    I am doing this in the cheapest possible way: I cast the pointer to
    an integer. This assumes that glui32 is big enough to hold an 
    arbitrary pointer. (If this is not true, don't even try to run this 
    code.)
   The first field of each object is a magic number which identifies the
    object type. This lets us have a *little* bit of run-time type 
    safety; if you try to convert an ID of the wrong type, the macro 
    will return NULL. But this is not very safe, because a random ID 
    value can still cause a memory fault or crash.
   A safer approach would be to store IDs in a hash table, and rewrite
    these macros to perform hash table lookups and references. Feel 
    free.
   Note that these macros are not intended to convert between the values
    NULL and 0. (It is true that WindowToID(NULL) will evaluate to 0 on
    almost all systems, but the library code does not rely on this, so
    if you rewrite WindowToID you don't need to guarantee it.) */

#define WindowToID(win)  ((glui32)(win))
#define StreamToID(str)  ((glui32)(str))
#define FilerefToID(fref)  ((glui32)(fref))

#define IDToWindow(id)    \
    ((((window_t *)(id))->magicnum == MAGIC_WINDOW_NUM)    \
    ? ((window_t *)(id)) : NULL)
#define IDToStream(id)    \
    ((((stream_t *)(id))->magicnum == MAGIC_STREAM_NUM)    \
    ? ((stream_t *)(id)) : NULL)
#define IDToFileref(id)    \
    ((((fileref_t *)(id))->magicnum == MAGIC_FILEREF_NUM)    \
    ? ((fileref_t *)(id)) : NULL)

/* This macro is called whenever the library code catches an error
    or illegal operation from the game program. */

#define gli_strict_warning(msg)   \
    (printf("Glk library error: %s\n", msg)) 

/* The overall screen size, as set by command-line options. A
    better implementation would check the real screen size
    somehow. */
extern int screenwidth, screenheight; 

/* Some useful type declarations. */

typedef struct window_struct window_t;
typedef struct stream_struct stream_t;
typedef struct fileref_struct fileref_t;

#define MAGIC_WINDOW_NUM (9876)
#define MAGIC_STREAM_NUM (8769)
#define MAGIC_FILEREF_NUM (7698)

struct window_struct {
    glui32 magicnum;
    glui32 rock;
    
    stream_t *str; /* the window stream. */
    stream_t *echostr; /* the window's echo stream, if any. */
    
    int line_request;
    int char_request;
    
    void *linebuf;
    glui32 linebuflen;
};

#define strtype_File (1)
#define strtype_Window (2)
#define strtype_Memory (3)

struct stream_struct {
    glui32 magicnum;
    glui32 rock;

    int type; /* file, window, or memory stream */
    
    glui32 readcount, writecount;
    int readable, writable;
    
    /* for strtype_Window */
    window_t *win;
    
    /* for strtype_File */
    FILE *file; 
    
    /* for strtype_Memory */
    unsigned char *buf;
    unsigned char *bufptr;
    unsigned char *bufend;
    unsigned char *bufeof;
    glui32 buflen;

    stream_t *next; /* in the big linked list of streams */
};

struct fileref_struct {
    glui32 magicnum;
    glui32 rock;

    char *filename;
    int filetype;
    int textmode;

    fileref_t *next; /* in the big linked list of filerefs */
};

/* Declarations of library internal functions. */

extern void gli_initialize_misc(void);

extern window_t *gli_new_window(glui32 rock);
extern void gli_delete_window(window_t *win);
extern window_t *gli_window_get(void);

extern stream_t *gli_new_stream(int type, int readable, int writable, 
    glui32 rock);
extern void gli_delete_stream(stream_t *str);
extern void gli_stream_set_current(stream_t *str);
extern void gli_stream_fill_result(stream_t *str, 
    stream_result_t *result);
extern void gli_stream_echo_line(stream_t *str, char *buf, glui32 len);

extern fileref_t *gli_new_fileref(char *filename, glui32 usage, 
    glui32 rock);
extern void gli_delete_fileref(fileref_t *fref);

/* A macro that I can't think of anywhere else to put it. */

#define gli_event_clearevent(evp)  \
    ((evp)->type = evtype_None,    \
    (evp)->win = 0,    \
    (evp)->val1 = 0,   \
    (evp)->val2 = 0)

#endif /* CHEAPGLK_H */
