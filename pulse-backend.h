#ifndef PULSE_BACKEND_H
#define PULSE_BACKEND_H

#include <pulse/pulseaudio.h>

typedef struct pa_devicelist {
    uint8_t initialized;
    char name[512];
    uint32_t index;
    char description[256];
} pa_devicelist_t;

struct pcmframe
{
	short l;
	short r;
};

int get_pulse_devices(pa_devicelist_t **sources);
gpointer pulse_input(gpointer data);
void disconnect();
int connection_active();

#endif
