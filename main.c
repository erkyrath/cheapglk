#include <stdio.h>
#include <stdlib.h>
#include "glk.h"
#include "cheapglk.h"

int screenwidth = 80;
int screenheight = 24; 

int main(int argc, char *argv[])
{
    int ix, val;
    int errflag = 0;
    
    /* Test for compile-time errors. If one of these spouts off, you
        must edit glk.h and recompile. */
    if (sizeof(glui32) != 4) {
        printf("Compile-time error: glui32 is not a 32-bit value. Please fix glk.h.\n");
        return 1;
    }
    if ((glui32)(-1) < 0) {
        printf("Compile-time error: glui32 is not unsigned. Please fix glk.h.\n");
        return 1;
    }
    
    /* Suck out -w WIDTH and -h HEIGHT arguments. */
    
    for (ix=1; ix<argc; ix++) {
        if (argv[ix][0] == '-') {
            switch (argv[ix][1]) {
                case 'w':
                    val = 0;
                    if (argv[ix][2]) 
                        val = atoi(argv[ix]+2);
                    else {
                        ix++;
                        if (ix<argc) 
                            val = atoi(argv[ix+1]);
                    }
                    if (val < 8)
                        errflag = 1;
                    else
                        screenwidth = val;
                    break;
                case 'h':
                    val = 0;
                    if (argv[ix][2]) 
                        val = atoi(argv[ix]+2);
                    else {
                        ix++;
                        if (ix<argc) 
                            val = atoi(argv[ix+1]);
                    }
                    if (val < 2)
                        errflag = 1;
                    else
                        screenheight = val;
                    break;
            }
        }
    }

    if (errflag) {
        printf("usage: %s -w WIDTH -h HEIGHT\n", argv[0]);
        return 1;
    }
    
    /* Initialize things. */
    gli_initialize_misc();
    
    printf("Welcome to the Cheap Glk Implementation, library version %s.\n\n", 
        LIBRARY_VERSION);
    glk_main();
    glk_exit();
    
    /* glk_exit() doesn't return, but the compiler may kvetch if main()
        doesn't seem to return a value. */
    return 0;
}
