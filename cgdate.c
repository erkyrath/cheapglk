#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "glk.h"
#include "cheapglk.h"

#ifdef GLK_MODULE_DATETIME

void glk_current_time(glktimeval_t *time)
{
}

glsi32 glk_current_simple_time(glui32 factor)
{
}

void glk_time_to_date_utc(glktimeval_t *time, glkdate_t *date)
{
}

void glk_time_to_date_local(glktimeval_t *time, glkdate_t *date)
{
}

void glk_simple_time_to_date_utc(glsi32 time, glui32 factor, 
    glkdate_t *date)
{
}

void glk_simple_time_to_date_local(glsi32 time, glui32 factor, 
    glkdate_t *date)
{
}

void glk_date_to_time_utc(glkdate_t *date, glktimeval_t *time)
{
}

void glk_date_to_time_local(glkdate_t *date, glktimeval_t *time)
{
}

glsi32 glk_date_to_simple_time_utc(glkdate_t *date, glui32 factor)
{
}

glsi32 glk_date_to_simple_time_local(glkdate_t *date, glui32 factor)
{
}



#endif /* GLK_MODULE_DATETIME */
