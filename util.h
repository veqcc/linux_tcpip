
#include <stdint.h>
#define min(x, y) ((x) < (y) ? (x) : (y))

extern void hexdump(FILE *fp, void *data, size_t size);
uint16_t hton16(uint16_t);
uint16_t ntoh16(uint16_t);