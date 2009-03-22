#ifndef CHEAPGLK_H
#define CHEAPGLK_H

/* cheapglk.h: Private header file for Cheapass Implementation of the 
        Glk API.
    CheapGlk Library: version 0.8.7.
    Glk API which this implements: version 0.6.1.
    Designed by Andrew Plotkin <erkyrath@eblong.com>
    http://www.eblong.com/zarf/glk/index.html
*/

#define LIBRARY_VERSION "0.8.7"

#include "gi_dispa.h"

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

/* This macro is called whenever the library code catches an error
    or illegal operation from the game program. */

#define gli_strict_warning(msg)   \
    (printf("Glk library error: %s\n", msg)) 

/* The overall screen size, as set by command-line options. A
    better implementation would check the real screen size
    somehow. */
extern int gli_screenwidth, gli_screenheight; 

/* Callbacks necessary for the dispatch layer. */
extern gidispatch_rock_t (*gli_register_obj)(void *obj, glui32 objclass);
extern void (*gli_unregister_obj)(void *obj, glui32 objclass, 
    gidispatch_rock_t objrock);
extern gidispatch_rock_t (*gli_register_arr)(void *array, glui32 len, 
    char *typecode);
extern void (*gli_unregister_arr)(void *array, glui32 len, char *typecode, 
    gidispatch_rock_t objrock);

/* Some useful type declarations. */

typedef struct glk_window_struct window_t;
typedef struct glk_stream_struct stream_t;
typedef struct glk_fileref_struct fileref_t;

#define MAGIC_WINDOW_NUM (9876)
#define MAGIC_STREAM_NUM (8769)
#define MAGIC_FILEREF_NUM (7698)

struct glk_window_struct {
    glui32 magicnum;
    glui32 rock;
    gidispatch_rock_t disprock;
    
    stream_t *str; /* the window stream. */
    stream_t *echostr; /* the window's echo stream, if any. */
    
    int line_request;
    int char_request;
    
    void *linebuf;
    glui32 linebuflen;
    gidispatch_rock_t inarrayrock;
};

#define strtype_File (1)
#define strtype_Window (2)
#define strtype_Memory (3)

struct glk_stream_struct {
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
    gidispatch_rock_t arrayrock;

    gidispatch_rock_t disprock;
    stream_t *next, *prev; /* in the big linked list of streams */
};

struct glk_fileref_struct {
    glui32 magicnum;
    glui32 rock;

    char *filename;
    int filetype;
    int textmode;

    gidispatch_rock_t disprock;
    fileref_t *next, *prev; /* in the big linked list of filerefs */
};

/* Declarations of library internal functions. */

extern void gli_initialize_misc(void);

extern window_t *gli_new_window(glui32 rock);
extern void gli_delete_window(window_t *win);
extern window_t *gli_window_get(void);

extern stream_t *gli_new_stream(int type, int readable, int writable, 
    glui32 rock);
extern void gli_delete_stream(stream_t *str);
extern strid_t gli_stream_open_pathname(char *pathname, int textmode,
    glui32 rock);
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
    (evp)->win = NULL,    \
    (evp)->val1 = 0,   \
    (evp)->val2 = 0)

#endif /* CHEAPGLK_H */
