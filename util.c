
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

void hexdump (FILE *fp, void *data, size_t size) {
    unsigned char *src = (unsigned char *) data;
    fprintf(fp, "+------+-------------------------------------------------+------------------+\n");
    for (int offset = 0; offset < (int) size; offset += 16) {
        fprintf(fp, "| %04x | ", offset);
        for (int index = 0; index < 16; index++) {
            if (offset + index < (int) size) {
                fprintf(fp, "%02x ", 0xff & src[offset + index]);
            } else {
                fprintf(fp, "   ");
            }
        }

        fprintf(fp, "| ");
        for (int index = 0; index < 16; index++) {
            if (offset + index < (int) size) {
                if (isascii(src[offset + index]) && isprint(src[offset + index])) {
                    fprintf(fp, "%c", src[offset + index]);
                } else {
                    fprintf(fp, ".");
                }
            } else {
                fprintf(fp, " ");
            }
        }

        fprintf(fp, " |\n");
    }

    fprintf(fp, "+------+-------------------------------------------------+------------------+\n");
}

#define __BIG_ENDIAN 4321
#define __LITTLE_ENDIAN 1234

static int endian;

int byteorder (void) {
    uint32_t x = 0x00000001;
    return *(uint8_t *)&x ? __LITTLE_ENDIAN : __BIG_ENDIAN;
}

uint16_t byteswap16 (uint16_t v) {
    return (v & 0x00ff) << 8 | (v & 0xff00 ) >> 8;
}

uint32_t byteswap32 (uint32_t v) {
    return (v & 0x000000ff) << 24 | (v & 0x0000ff00) << 8 | (v & 0x00ff0000) >> 8 | (v & 0xff000000) >> 24;
}

uint16_t hton16 (uint16_t h) {
    if (!endian) {
        endian = byteorder();
    }
    return endian == __LITTLE_ENDIAN ? byteswap16(h) : h;
}

uint16_t ntoh16 (uint16_t n) {
    if (!endian) {
        endian = byteorder();
    }
    return endian == __LITTLE_ENDIAN ? byteswap16(n) : n;
}