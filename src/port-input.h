#ifndef PORT_INPUT_H 
#define PORT_INPUT_H 

#include <portaudio.h>
#include "synesthesia-app.h"

int port_init(void);
int port_kill(void);
int get_port_devices (const PaDeviceInfo **devicelist[]);
gpointer port_input(gpointer data);
pcmframe *port_getbuffer(int *count, int *clear);
void port_disconnect();
int port_connection_active();

#endif
