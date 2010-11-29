#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "glk.h"
#include "cheapglk.h"

void gli_putchar_utf8(glui32 val, FILE *fl)
{
    if (val < 0x80) {
        putc(val, fl);
    }
    else if (val < 0x800) {
        putc((0xC0 | ((val & 0x7C0) >> 6)), fl);
        putc((0x80 |  (val & 0x03F)     ),  fl);
    }
    else if (val < 0x10000) {
        putc((0xE0 | ((val & 0xF000) >> 12)), fl);
        putc((0x80 | ((val & 0x0FC0) >>  6)), fl);
        putc((0x80 |  (val & 0x003F)      ),  fl);
    }
    else if (val < 0x200000) {
        putc((0xF0 | ((val & 0x1C0000) >> 18)), fl);
        putc((0x80 | ((val & 0x03F000) >> 12)), fl);
        putc((0x80 | ((val & 0x000FC0) >>  6)), fl);
        putc((0x80 |  (val & 0x00003F)      ),  fl);
    }
    else {
        putc('?', fl);
    }
}

glui32 gli_parse_utf8(unsigned char *buf, glui32 buflen,
    glui32 *out, glui32 outlen)
{
    glui32 pos = 0;
    glui32 outpos = 0;
    glui32 res;
    glui32 val0, val1, val2, val3;

    while (outpos < outlen) {
        if (pos >= buflen)
            break;

        val0 = buf[pos++];

        if (val0 < 0x80) {
            res = val0;
            out[outpos++] = res;
            continue;
        }

        if ((val0 & 0xe0) == 0xc0) {
            if (pos+1 > buflen) {
                gli_strict_warning("incomplete two-byte character");
                break;
            }
            val1 = buf[pos++];
            if ((val1 & 0xc0) != 0x80) {
                gli_strict_warning("malformed two-byte character");
                break;
            }
            res = (val0 & 0x1f) << 6;
            res |= (val1 & 0x3f);
            out[outpos++] = res;
            continue;
        }

        if ((val0 & 0xf0) == 0xe0) {
            if (pos+2 > buflen) {
                gli_strict_warning("incomplete three-byte character");
                break;
            }
            val1 = buf[pos++];
            val2 = buf[pos++];
            if ((val1 & 0xc0) != 0x80) {
                gli_strict_warning("malformed three-byte character");
                break;
            }
            if ((val2 & 0xc0) != 0x80) {
                gli_strict_warning("malformed three-byte character");
                break;
            }
            res = (((val0 & 0xf)<<12)  & 0x0000f000);
            res |= (((val1 & 0x3f)<<6) & 0x00000fc0);
            res |= (((val2 & 0x3f))    & 0x0000003f);
            out[outpos++] = res;
            continue;
        }

        if ((val0 & 0xf0) == 0xf0) {
            if ((val0 & 0xf8) != 0xf0) {
                gli_strict_warning("malformed four-byte character");
                break;        
            }
            if (pos+3 > buflen) {
                gli_strict_warning("incomplete four-byte character");
                break;
            }
            val1 = buf[pos++];
            val2 = buf[pos++];
            val3 = buf[pos++];
            if ((val1 & 0xc0) != 0x80) {
                gli_strict_warning("malformed four-byte character");
                break;
            }
            if ((val2 & 0xc0) != 0x80) {
                gli_strict_warning("malformed four-byte character");
                break;
            }
            if ((val3 & 0xc0) != 0x80) {
                gli_strict_warning("malformed four-byte character");
                break;
            }
            res = (((val0 & 0x7)<<18)   & 0x1c0000);
            res |= (((val1 & 0x3f)<<12) & 0x03f000);
            res |= (((val2 & 0x3f)<<6)  & 0x000fc0);
            res |= (((val3 & 0x3f))     & 0x00003f);
            out[outpos++] = res;
            continue;
        }

        gli_strict_warning("malformed character");
    }

    return outpos;
}

#ifdef GLK_MODULE_UNICODE

#include "cgunigen.c"

#define CASE_UPPER (0)
#define CASE_LOWER (1)
#define CASE_TITLE (2)
#define CASE_IDENT (3)

#define COND_ALL (0)
#define COND_LINESTART (1)

static glui32 gli_buffer_change_case(glui32 *buf, glui32 len,
    glui32 numchars, int destcase, int cond, int changerest)
{
    glui32 ix, jx;
    glui32 *outbuf;
    glui32 *newoutbuf;
    glui32 outcount;
    int dest_block_rest, dest_block_first;
    int dest_spec_rest, dest_spec_first;

    switch (cond) {
    case COND_ALL:
        dest_spec_rest = destcase;
        dest_spec_first = destcase;
        break;
    case COND_LINESTART:
        if (changerest)
            dest_spec_rest = CASE_LOWER;
        else
            dest_spec_rest = CASE_IDENT;
        dest_spec_first = destcase;
        break;
    }

    dest_block_rest = dest_spec_rest;
    if (dest_block_rest == CASE_TITLE)
        dest_block_rest = CASE_UPPER;
    dest_block_first = dest_spec_first;
    if (dest_block_first == CASE_TITLE)
        dest_block_first = CASE_UPPER;

    newoutbuf = NULL;
    outcount = 0;
    outbuf = buf;

    for (ix=0; ix<numchars; ix++) {
        int target;
        int isfirst;
        glui32 res;
        glui32 *special;
        glui32 *ptr;
        glui32 speccount;
        glui32 ch = buf[ix];

        isfirst = (ix == 0);
        
        target = (isfirst ? dest_block_first : dest_block_rest);

        if (target == CASE_IDENT) {
            res = ch;
        }
        else {
            gli_case_block_t *block;

            GET_CASE_BLOCK(ch, &block);
            if (!block)
                res = ch;
            else
                res = block[ch & 0xFF][target];
        }

        if (res != 0xFFFFFFFF || res == ch) {
            /* simple case */
            if (outcount < len)
                outbuf[outcount] = res;
            outcount++;
            continue;
        }

        target = (isfirst ? dest_spec_first : dest_spec_rest);

        /* complicated cases */
        GET_CASE_SPECIAL(ch, &special);
        if (!special) {
            gli_strict_warning("inconsistency in cgunigen.c");
            continue;
        }
        ptr = &unigen_special_array[special[target]];
        speccount = *(ptr++);
        
        if (speccount == 1) {
            /* simple after all */
            if (outcount < len)
                outbuf[outcount] = ptr[0];
            outcount++;
            continue;
        }

        /* Now we have to allocate a new buffer, if we haven't already. */
        if (!newoutbuf) {
            newoutbuf = malloc((len+1) * sizeof(glui32));
            if (!newoutbuf)
                return 0;
            if (outcount)
                memcpy(newoutbuf, buf, outcount * sizeof(glui32));
            outbuf = newoutbuf;
        }

        for (jx=0; jx<speccount; jx++) {
            if (outcount < len)
                outbuf[outcount] = ptr[jx];
            outcount++;
        }
    }

    if (newoutbuf) {
        if (outcount)
            memcpy(buf, newoutbuf, outcount * sizeof(glui32));
        free(newoutbuf);
    }

    return outcount;
}

glui32 glk_buffer_to_lower_case_uni(glui32 *buf, glui32 len,
    glui32 numchars)
{
    return gli_buffer_change_case(buf, len, numchars, 
        CASE_LOWER, COND_ALL, TRUE);
}

glui32 glk_buffer_to_upper_case_uni(glui32 *buf, glui32 len,
    glui32 numchars)
{
    return gli_buffer_change_case(buf, len, numchars, 
        CASE_UPPER, COND_ALL, TRUE);
}

glui32 glk_buffer_to_title_case_uni(glui32 *buf, glui32 len,
    glui32 numchars, glui32 lowerrest)
{
    return gli_buffer_change_case(buf, len, numchars, CASE_TITLE, 
        COND_LINESTART, lowerrest);
}

#endif /* GLK_MODULE_UNICODE */

#ifdef GLK_MODULE_UNICODE_NORM

/* We're relying on the fact that cgunigen.c has already been included.
   So don't try to use GLK_MODULE_UNICODE_NORM without GLK_MODULE_UNICODE.
*/

static glui32 combining_class(glui32 ch)
{
    RETURN_COMBINING_CLASS(ch);
}

glui32 glk_buffer_canon_decompose_uni(glui32 *buf, glui32 len,
    glui32 numchars)
{
    /* The algorithm for the canonical decomposition of a string: For
       each character, look up the decomposition in the decomp table.
       Append the decomposition to the buffer. Finally, sort every
       substring of the buffer which is made up of combining
       characters (characters with a nonzero combining class). */

    glui32 *dest = buf;
    glui32 destsize = 0;
    glui32 destlen = 0;
    glui32 ix, jx;
    int anycombining = FALSE;

    /* We do this in place if at all possible. */

    for (ix=0; ix<numchars; ix++) {
        glui32 ch = buf[ix];
        gli_decomp_block_t *block;
        glui32 count, pos;

        if (combining_class(ch))
            anycombining = TRUE;

        GET_DECOMP_BLOCK(ch, &block);
        if (block) {
            block += (ch & 0xFF);
            count = (*block)[0];
            pos = (*block)[1];
        }
        else {
            GET_DECOMP_SPECIAL(ch, &count, &pos);
        }

        if (!count) {
            /* The simple case: this character doesn't decompose. Push
               it straight into the destination, unless we don't have
               a destination buffer, in which case just advance a
               character. */
            if (dest != buf) {
                if (destlen >= destsize) {
                    destsize = destsize * 2;
                    dest = (glui32 *)realloc(dest, destsize * sizeof(glui32));
                    if (!dest)
                        return 0;
                }
                dest[destlen] = ch;
            }
            destlen++;
            continue;
        }

        /* Assume that a character with a decomposition has a
           combining class somewhere in there. Not always true, but
           it's simpler to assume it. */
        anycombining = TRUE;

        /* We now append count characters to the buffer, reading from
           unigen_decomp_data[pos] onwards. None of these characters
           are decomposable; that was already recursively expanded when
           unigen_decomp_data was generated. */

        if (dest == buf) {
            /* Time to allocate a separate destination buffer. We
               allow space equal to twice the original string length,
               plus some. That's almost certainly enough. */
            destsize = len * 2 + 16;
            dest = (glui32 *)malloc(destsize * sizeof(glui32));
            if (!dest)
                return 0;
            if (destlen)
                memcpy(dest, buf, destlen * sizeof(glui32));
        }
        if (destlen+count >= destsize) {
            /* Okay, that wasn't enough. Expand more. */
            destsize = destsize * 2 + count;
            dest = (glui32 *)realloc(dest, destsize * sizeof(glui32));
            if (!dest)
                return 0;
        }
        for (jx=0; jx<count; jx++) {
            dest[destlen] = unigen_decomp_data[pos+jx];
            destlen++;
        }
    }

    if (anycombining) {
        /* Now we sort groups of combining characters. This should be a
           stable sort by the combining-class number. We're lazy and
           nearly all groups are short, so we'll just bubble-sort. */
        glui32 grpstart, grpend, kx;
        ix = 0;
        while (ix < destlen) {
            if (!combining_class(dest[ix])) {
                ix++;
                continue;
            }
            grpstart = ix;
            while (ix < destlen && combining_class(dest[ix])) 
                ix++;
            grpend = ix;
            if (grpend - grpstart >= 2) {
                /* Sort this group. */
                for (jx = grpend-1; jx > grpstart; jx--) {
                    for (kx = grpstart; kx < jx; kx++) {
                        if (combining_class(dest[kx]) > combining_class(dest[kx+1])) {
                            glui32 tmp = dest[kx];
                            dest[kx] = dest[kx+1];
                            dest[kx+1] = tmp;
                        }
                    }
                }
            }
        }
    }

    if (dest != buf) {
        /* If we were forced to allocate a separate buffer, copy the
           data back. */
        ix = destlen;
        if (ix > len)
            ix = len;
        if (ix)
            memcpy(buf, dest, ix * sizeof(glui32));
    }

    return destlen;
}

glui32 glk_buffer_canon_normalize_uni(glui32 *buf, glui32 len,
    glui32 numchars)
{
    return 0; /*###*/
}

#endif /* GLK_MODULE_UNICODE_NORM */

