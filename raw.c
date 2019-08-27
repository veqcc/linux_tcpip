
#include <stdio.h>
#include <stdlib.h>

#include "raw.h"
#include "raw/soc.h"

extern struct rawdev_ops soc_dev_ops;

struct rawdev *rawdev_alloc(uint8_t type, char *name) {
    struct rawdev *raw = malloc(sizeof(struct rawdev));
    if (!raw) {
        fprintf(stderr, "malloc: failure\n");
        return NULL;
    }

    raw->type = type;
    raw->name = name;
    raw->ops = &soc_dev_ops;
    raw->private = NULL;

    return raw;
}