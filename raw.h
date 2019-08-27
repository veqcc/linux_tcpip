
#include <stddef.h>
#include <stdint.h>
#include <net/if.h>

#define RAWDEV_TYPE_SOCKET 2

struct rawdev;

struct rawdev_ops {
    int (*open)(struct rawdev *raw);
    void (*close)(struct rawdev *raw);
    void (*rx)(struct rawdev *raw,
            void (*callback)(uint8_t *, size_t, void *), void *arg, int timeout);
    ssize_t (*tx)(struct rawdev *raw, const uint8_t *buf, size_t len);
    int (*addr)(struct rawdev *raw, uint8_t *dst, size_t len);
};

struct rawdev {
    uint8_t type;
    char *name;
    struct rawdev_ops *ops;
    void *private;
};

struct rawdev *rawdev_alloc(uint8_t type, char *name);