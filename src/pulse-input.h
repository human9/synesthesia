#ifndef PULSE_BACKEND_H
#define PULSE_BACKEND_H

#include <pulse/pulseaudio.h>
#include "synesthesia-app.h"

typedef struct pa_devicelist {
    uint8_t initialized;
    char name[512];
    uint32_t index;
    char description[256];
} pa_devicelist_t;

int get_pulse_devices(pa_devicelist_t **sources);
gpointer pulse_input(gpointer data);
void pulse_disconnect();
pcmframe *pulse_getbuffer(int *count, int *clear);
int pulse_connection_active();

#endif
