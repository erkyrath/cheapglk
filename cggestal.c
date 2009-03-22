#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "glk.h"
#include "cheapglk.h"

glui32 glk_gestalt(glui32 id, glui32 val)
{
    return glk_gestalt_ext(id, val, NULL);
}

glui32 glk_gestalt_ext(glui32 id, glui32 val, void *ptr)
{
    int ix;
    
    switch (id) {
        
        case gestalt_Version:
            return 0x00000501;
        
        case gestalt_LineInput:
            if (val >= 32 && val < 127)
                return TRUE;
            else
                return FALSE;
                
        case gestalt_CharInput: 
            if (val >= 32 && val < 127)
                return TRUE;
            else if (val == keycode_Return)
                return TRUE;
            else
                return FALSE;
        
        case gestalt_CharOutput: 
            if (val >= 32 && val < 127) {
                if (ptr)
                    *((glui32 *)ptr) = 1;
                return gestalt_CharOutput_ExactPrint;
            }
            else {
                /* cheaply, we don't do any translation of printed
                    characters, so the output is always one character 
                    even if it's wrong. */
                if (ptr)
                    *((glui32 *)ptr) = 1;
                return gestalt_CharOutput_CannotPrint;
            }
            
        case gestalt_MouseInput: 
            return FALSE;
            
        case gestalt_Timer: 
            return FALSE;

        case gestalt_Graphics:
            return FALSE;
            
        case gestalt_DrawImage:
            return FALSE;
            
        case gestalt_Sound:
        case gestalt_SoundVolume:
        case gestalt_SoundNotify: 
	    return FALSE;

        default:
            return 0;

    }
}

