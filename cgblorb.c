#include "glk.h"
#include "gi_blorb.h"

/* This is merely a stub, for programs that try to link in Blorb support.
    Since CheapGlk does not support graphics or sounds, any attempt to
    set a resource map is just rejected.
*/

giblorb_err_t giblorb_set_resource_map(strid_t file)
{
    return giblorb_err_CompileTime;
}
