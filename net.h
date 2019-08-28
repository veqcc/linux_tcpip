
#include <stdint.h>
#include <stdio.h>

#define NETDEV_TYPE_ETHERNET 0x0001
#define NETDEV_FLAG_BROADCAST 0x0001
#define IF_NAME_SIZE 16

struct netdev;

struct netif {
    struct netif *next;
    uint8_t family;
    struct netdev *dev;
};

struct netdev_ops {
    int (*open)(struct netdev *dev, int op);
    int (*close)(struct netdev *dev);
    int (*run)(struct netdev *dev);
    int (*stop)(struct netdev *dev);
    ssize_t (*tx)(struct netdev *dev, uint16_t type, uint8_t *packet,
            size_t size, const void *dst);
};

struct netdev_def {
    uint16_t  type;
    uint16_t mtu;
    uint16_t flags;
    uint16_t hlen;
    uint16_t alen;
    struct netdev_ops *ops;
};

struct netdev {
    struct netdev *next;
    struct netif *ifs;
    char name[IF_NAME_SIZE];
    uint16_t type;
    uint16_t mtu;
    uint16_t flags;
    uint16_t hlen;
    uint16_t alen;
    uint8_t addr[16];
    uint8_t peer[16];
    uint8_t broadcast[16];
    void (*rx_handler)(struct netdev *dev, uint16_t type, uint8_t *packet, size_t plen);
    struct netdev_ops *ops;
    void *private;
};

int netdev_driver_register(struct netdev_def *def);
struct netdev *netdev_alloc(uint16_t type);