
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <poll.h>

#include "soc.h"

struct soc_dev {
    int fd;
};

struct soc_dev *soc_dev_open(char *name) {
    struct soc_dev *dev = malloc(sizeof(struct soc_dev));
    if (!dev) {
        fprintf(stderr, "malloc: failure\n");
        return NULL;
    }

    dev->fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (dev->fd == -1) {
        perror("socket");
        soc_dev_close(dev);
        return NULL;
    }

    // find device interface index from name
    struct ifreq ifr;
    strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name) - 1);
    if (ioctl(dev->fd, SIOCGIFINDEX, &ifr) == -1) {
        perror("ioctl");
        soc_dev_close(dev);
        return NULL;
    }

    // bind
    struct sockaddr_ll sockaddr;
    memset(&sockaddr, 0x00, sizeof(sockaddr));
    sockaddr.sll_family = AF_PACKET;
    sockaddr.sll_protocol = htons(ETH_P_ALL);
    sockaddr.sll_ifindex = ifr.ifr_ifindex;
    if (bind(dev->fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) == -1) {
        perror("bind");
        soc_dev_close(dev);
        return NULL;
    }

    // get flag
    if (ioctl(dev->fd, SIOCGIFFLAGS, &ifr) == -1) {
        perror("ioctl");
        soc_dev_close(dev);
        return NULL;
    }

    // set PROMISC flag
    ifr.ifr_flags |= IFF_PROMISC;
    if (ioctl(dev->fd, SIOCSIFFLAGS, &ifr) == -1) {
        perror("ioctl");
        soc_dev_close(dev);
        return NULL;
    }

    return dev;
}

void soc_dev_close(struct soc_dev *dev) {
    if (dev->fd != -1) {
        close(dev->fd);
    }
    free(dev);
}

void soc_dev_rx(struct soc_dev *dev,
        void (*callback)(uint8_t *, size_t, void *), void *arg, int timeout) {

    struct pollfd pfd;
    pfd.fd = dev->fd;
    pfd.events = POLLIN;

    int ret = poll(&pfd, 1, timeout);
    switch (ret) {
        case -1:
            if (errno != EINTR) {
                perror("poll");
            }
        case 0:
            return;
        default:
            break;
    }

    uint8_t buf[2048];
    ssize_t len = read(dev->fd, buf, sizeof(buf));
    switch (len) {
        case -1:
            perror("read");
        case 0:
            return;
        default:
            break;
    }

    callback(buf, len, arg);
}

ssize_t soc_dev_tx(struct soc_dev *dev, const uint8_t *buf, size_t len) {
    return write(dev->fd, buf, len);
}

int soc_dev_addr(char *name, uint8_t *dst, size_t size) {
    // any valid socket is ok. type doesnt matter.
    int fd = socket(AF_PACKET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }

    // set HWADDR (MAC address of NIC) in ifr
    struct ifreq ifr;
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name) - 1);
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1) {
        perror("ioctl");
        close(fd);
        return -1;
    }

    memcpy(dst, ifr.ifr_hwaddr.sa_data, size);
    close(fd);
    return 0;
}

#include "raw.h"


static int soc_dev_open_wrap(struct rawdev *dev) {
    dev->private = soc_dev_open(dev->name);
    return dev->private ? 0 : -1;
}

static void soc_dev_close_wrap(struct rawdev *dev) {
    soc_dev_close(dev->private);
}

static void soc_dev_rx_wrap(struct rawdev *dev,
        void (*callback)(uint8_t *, size_t, void *), void *arg, int timeout) {
    soc_dev_rx(dev->private, callback, arg, timeout);
}

static ssize_t soc_dev_tx_wrap(struct rawdev *dev, const uint8_t *buf, size_t len) {
    return soc_dev_tx(dev->private, buf, len);
}

static int soc_dev_addr_wrap(struct rawdev *dev, uint8_t *dst, size_t size) {
    return soc_dev_addr(dev->name, dst, size);
}

struct rawdev_ops soc_dev_ops = {
        .open = soc_dev_open_wrap,
        .close = soc_dev_close_wrap,
        .rx = soc_dev_rx_wrap,
        .tx = soc_dev_tx_wrap,
        .addr = soc_dev_addr_wrap,
};