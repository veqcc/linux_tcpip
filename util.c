
#include <stdio.h>
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
