
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "ethernet.h"
#include "net.h"
#include "raw.h"
#include "util.h"

struct ethernet_header {
    uint8_t dst[ETHERNET_ADDR_LEN];
    uint8_t src[ETHERNET_ADDR_LEN];
    uint16_t type;
};

struct ethernet_private {
    struct netdev *dev;
    struct rawdev *raw;
    pthread_t thread;
    int terminate;
};

const uint8_t ETHERNET_ADDR_ANY[ETHERNET_ADDR_LEN] = {"\x00\x00\x00\x00\x00\x00"};
const uint8_t ETHERNET_ADDR_BROADCAST[ETHERNET_ADDR_LEN] = {"\xff\xff\xff\xff\xff\xff"};

int ethernet_addr_pton(const char *p, uint8_t *n) {
    if (!p || !n) {
        return -1;
    }

    int index = 0;
    char *ep;
    for (; index < ETHERNET_ADDR_LEN; index++) {
        long val = strtol(p, &ep, 16);
        if (ep == p || val < 0 || val > 0xff || (index < ETHERNET_ADDR_LEN - 1 && *ep != ':')) {
            break;
        }
        n[index] = (uint8_t)val;
        p = ep + 1;
    }
    if (index != ETHERNET_ADDR_LEN || *ep != '\0') {
        return -1;
    }
    return 0;
}

char *ethernet_ntop(const uint8_t *n, char *p, size_t size) {
    if (!n || !p) {
        return NULL;
    }

    snprintf(p, size, "%02x:%02x:%02x:%02x:%02x:%02x", n[0], n[1], n[2], n[3], n[4], n[5]);

    return p;
}

int ethernet_open(struct netdev *dev, int opt) {
    struct ethernet_private *priv = malloc(sizeof(struct ethernet_private));
    if (!priv) {
        return -1;
    }

    struct rawdev *raw = rawdev_alloc(opt, dev->name);
    if (!raw) {
        free(priv);
        return -1;
    }

    if (raw->ops->open(raw) == -1) {
        free(raw);
        free(priv);
        return -1;
    }

    priv->raw = raw;
    priv->thread = pthread_self();
    priv->terminate = 0;
    priv->dev = dev;
    dev->private = priv;
    if (memcmp(dev->addr, ETHERNET_ADDR_ANY, ETHERNET_ADDR_LEN) == 0) {
        raw->ops->addr(raw, dev->addr, ETHERNET_ADDR_LEN);
    }

    memcpy(dev->broadcast, ETHERNET_ADDR_BROADCAST, ETHERNET_ADDR_LEN);
    return 0;
}

int ethernet_close(struct netdev *dev) {
    if (!dev || !dev->private) {
        return 1;
    }

    struct ethernet_private *priv = (struct ethernet_private *)dev->private;
    if (!pthread_equal(priv->thread, pthread_self())) {
        priv->terminate = 1;
        pthread_join(priv->thread, NULL);
    }

    if (priv->raw) {
        priv->raw->ops->close(priv->raw);
        priv->raw = NULL;
    }

    free(priv);
    dev->private = NULL;

    return 0;
}

static void ethernet_rx(uint8_t *frame, size_t flen, void *arg) {
    if (flen < sizeof(struct ethernet_header)) {
        fprintf(stderr, "ethernet_rx: frame is too short\n");
        return;
    }

    struct netdev *dev = (struct netdev *)arg;
    struct ethernet_header *hdr = (struct ethernet_header *)frame;
    if (memcmp(dev->addr, hdr->dst, ETHERNET_ADDR_LEN) != 0) {
        if (memcmp(ETHERNET_ADDR_BROADCAST, hdr->dst, ETHERNET_ADDR_LEN) != 0) {
            return;
        }
    }

    fprintf(stderr, ">>> ethernet_rx <<<\n");

    uint8_t *payload = (uint8_t *)(hdr + 1);
    size_t plen = flen - sizeof(struct ethernet_header);
    dev->rx_handler(dev, hdr->type, payload, plen);
}

static void *ethernet_rx_thread(void *arg) {
    struct netdev *dev = (struct netdev *)arg;
    struct ethernet_private *priv = (struct ethernet_private *)dev->private;
    while (!priv->terminate) {
        priv->raw->ops->rx(priv->raw, ethernet_rx, dev, 1000);
    }
    return NULL;
}

int ethernet_run(struct netdev *dev) {
    struct ethernet_private *priv = (struct ethernet_private *)dev->private;
    if (pthread_create(&priv->thread, NULL, ethernet_rx_thread, dev) != 0) {
        fprintf(stderr, "ethernet_run: pthread_create: error\n");
        return -1;
    }
    return 0;
}

int ethernet_stop(struct netdev *dev) {
    struct ethernet_private *priv = (struct ethernet_private *)dev->private;
    priv->terminate = 1;
    pthread_join(priv->thread, NULL);
    priv->thread = pthread_self();
    priv->terminate = 0;
    return 0;
}

ssize_t ethernet_tx(struct netdev *dev, uint16_t type, uint8_t *payload,
        size_t plen, const void *dst) {

    if (!payload || plen > ETHERNET_PAYLOAD_SIZE_MAX || !dst) {
        return -1;
    }

    uint8_t frame[ETHERNET_FRAME_SIZE_MAX];
    memset(frame, 0, sizeof(frame));

    struct ethernet_header *hdr = (struct ethernet_header *)frame;
    memcpy(hdr->dst, dst, ETHERNET_ADDR_LEN);
    memcpy(hdr->src, dev->addr, ETHERNET_ADDR_LEN);
    hdr->type = hton16(type);
    memcpy(hdr + 1, payload, plen);
    size_t flen = sizeof(struct ethernet_header) + min(plen, ETHERNET_PAYLOAD_SIZE_MIN);

    fprintf(stderr, ">>> ethernet tx <<<\n");

    struct ethernet_private *priv = (struct ethernet_private *)dev->private;
    return priv->raw->ops->tx(priv->raw, frame, flen) == (ssize_t)flen ? (ssize_t)plen : -1;
}

struct netdev_ops ethernet_ops = {
        .open = ethernet_open,
        .close = ethernet_close,
        .run = ethernet_run,
        .stop = ethernet_stop,
        .tx = ethernet_tx,
};

struct netdev_def ethernet_def = {
        .type = NETDEV_TYPE_ETHERNET,
        .mtu = ETHERNET_PAYLOAD_SIZE_MAX,
        .flags = NETDEV_FLAG_BROADCAST,
        .hlen = ETHERNET_HDR_SIZE,
        .alen = ETHERNET_ADDR_LEN,
        .ops = &ethernet_ops,
};

int ethernet_init() {
    if (netdev_driver_register(&ethernet_def) == -1) {
        return -1;
    }
    return 0;
}