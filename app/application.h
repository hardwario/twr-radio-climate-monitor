#ifndef _APPLICATION_H
#define _APPLICATION_H

#ifndef TAG
#define TAG "vdev"
#endif

#include <bcl.h>

typedef struct
{
    uint8_t number;
    float value;
    bc_tick_t next_pub;

} event_param_t;

#endif // _APPLICATION_H
