
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include "ethernet.h"
#include "net.h"
#include "raw.h"

static int setup() {
    if (ethernet_init() == -1) {
        fprintf(stderr, "ethernet_init(): failure\n");
        return -1;
    }
    return 0;
}

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s interface\n", argv[0]);
        return -1;
    }

    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigprocmask(SIG_BLOCK, &sigset, NULL);
    if (setup() == -1) {
        return -1;
    }

    struct netdev *dev = netdev_alloc(NETDEV_TYPE_ETHERNET);
    if (!dev) {
        fprintf(stderr, "netdev_alloc() : failed\n");
        return -1;
    }

    strncpy(dev->name, argv[1], sizeof(dev->name) - 1);
    if (dev->ops->open(dev, RAWDEV_TYPE_SOCKET) == -1) {
        fprintf(stderr, "failed to open raw device\n");
        return -1;
    }

    dev->ops->run(dev);

    int signo;
    while (1) {
        sigwait(&sigset, &signo);
        if (signo == SIGINT) {
            break;
        }
    }

    if (dev->ops->close) {
        dev->ops->close(dev);
    }

    fprintf(stderr, "closed\n");

    return 0;
}