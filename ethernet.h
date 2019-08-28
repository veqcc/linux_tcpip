
#include <stddef.h>
#include <stdint.h>

#define ETHERNET_ADDR_LEN 6
#define ETHERNET_HDR_SIZE 14
#define ETHERNET_TRL_SIZE 4
#define ETHERNET_FRAME_SIZE_MIN 64
#define ETHERNET_FRAME_SIZE_MAX 1518
#define ETHERNET_PAYLOAD_SIZE_MIN (ETHERNET_FRAME_SIZE_MIN - (ETHERNET_HDR_SIZE + ETHERNET_TRL_SIZE))
#define ETHERNET_PAYLOAD_SIZE_MAX (ETHERNET_FRAME_SIZE_MAX - (ETHERNET_HDR_SIZE + ETHERNET_TRL_SIZE))

int ethernet_addr_pton(const char *p, uint8_t *n);
char *ethernet_addr_ntop(const uint8_t *n, char *p, size_t size);
int ethernet_init();