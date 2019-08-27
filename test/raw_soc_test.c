
#include <stdio.h>
#include <signal.h>

#include "../raw/soc.h"

volatile sig_atomic_t terminate;

static void on_signal(int s) {
    terminate = 1;
}

static void rx_handler(uint8_t *fname, size_t len, void *arg) {
    fprintf(stderr, "receive %zu octets\n", len);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, on_signal);

    if (argc != 2) {
        fprintf(stderr, "usage: %s device\n", argv[0]);
        return -1;
    }

    char *name = argv[1];
    struct soc_dev *dev = soc_dev_open(name);
    if (!dev) {
        return -1;
    }

    uint8_t addr[6];
    soc_dev_addr(name, addr, sizeof(addr));
    fprintf(stderr, "[%s] %02x:%02x:%02x:%02x:%02x:%02x\n",
            name, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    while (!terminate) {
        soc_dev_rx(dev, rx_handler, dev, 1000);
    }

    soc_dev_close(dev);
    return 0;
}