// https://dev.to/rdentato/utf-8-strings-in-c-1-3-42a4

#include <errno.h>
#include "stdint.h"

typedef uint32_t u8chr_t;

static uint8_t const u8_length[] = {
// 0 1 2 3 4 5 6 7 8 9 A B C D E F
        1,1,1,1,1,1,1,1,0,0,0,0,2,2,3,4
};

#define u8length(s) u8_length[(((uint8_t *)(s))[0] & 0xFF) >> 4];

/**
 * valid utf-8 char
 * @param c
 * @return
 */
int u8chrisvalid(u8chr_t c)
{
    if (c <= 0x7F) return 1;                    // [1]

    if (0xC280 <= c && c <= 0xDFBF)             // [2]
        return ((c & 0xE0C0) == 0xC080);

    if (0xEDA080 <= c && c <= 0xEDBFBF)         // [3]
        return 0; // Reject UTF-16 surrogates

    if (0xE0A080 <= c && c <= 0xEFBFBF)         // [4]
        return ((c & 0xF0C0C0) == 0xE08080);

    if (0xF0908080 <= c && c <= 0xF48FBFBF)     // [5]
        return ((c & 0xF8C0C0C0) == 0xF0808080);

    return 0;
}

/**
 * next utf-8 char
 * @param txt
 * @param ch
 * @return
 */
size_t u8next(unsigned char *txt, u8chr_t *ch)
{
    size_t len;
    u8chr_t encoding = 0;

    len = u8length(txt);

    for (int i=0; i<len && txt[i] != '\0'; i++) {
        encoding = (encoding << 8) | txt[i];
    }

    errno = 0;
    if (len == 0 || !u8chrisvalid(encoding)) {
        encoding = txt[0];
        len = 1;
        errno = EINVAL;
    }

    if (ch) *ch = encoding;

    return encoding ? len : 0 ;
}


// Returns the number of characters in an UTF-8 encoded string.
// (Does not check for encoding validity)
size_t u8strlen(const char *s)
{
    size_t len=0;

    while (*s) {

        size_t w = u8next((unsigned char *) s, NULL);

        if (w == 0) {

            break;
        }

        s+= w;
        len++;
    }

    return len;
}

/**
 * Avoids truncating multibyte UTF-8 encoding at the end.
 */

char *u8strncpy(char *dest, const char *src, size_t n)
{
    int k = n-1;
    int i;
    if (n) {
        dest[k] = 0;
        strncpy(dest,src,n);
        if (dest[k] & 0x80) { // Last byte has been overwritten
            for (i=k; (i>0) && ((k-i) < 3) && ((dest[i] & 0xC0) == 0x80); i--) ;
            switch(k-i) {
                case 0:                                 dest[i] = '\0'; break;
                case 1:  if ( (dest[i] & 0xE0) != 0xC0) dest[i] = '\0'; break;
                case 2:  if ( (dest[i] & 0xF0) != 0xE0) dest[i] = '\0'; break;
                case 3:  if ( (dest[i] & 0xF8) != 0xF0) dest[i] = '\0'; break;
            }
        }
    }
    return dest;
}

// from UTF-8 encoding to Unicode Codepoint
uint32_t u8decode(u8chr_t c)
{
    uint32_t mask;

    if (c > 0x7F) {
        mask = (c <= 0x00EFBFBF)? 0x000F0000 : 0x003F0000 ;
        c = ((c & 0x07000000) >> 6) |
            ((c & mask )      >> 4) |
            ((c & 0x00003F00) >> 2) |
            (c & 0x0000003F);
    }

    return c;
}

// From Unicode Codepoint to UTF-8 encoding
u8chr_t u8encode(uint32_t codepoint)
{
    u8chr_t c = codepoint;

    if (codepoint > 0x7F) {
        c =  (codepoint & 0x000003F)
             | (codepoint & 0x0000FC0) << 2
             | (codepoint & 0x003F000) << 4
             | (codepoint & 0x01C0000) << 6;

        if      (codepoint < 0x0000800) c |= 0x0000C080;
        else if (codepoint < 0x0010000) c |= 0x00E08080;
        else                            c |= 0xF0808080;
    }
    return c;
}
