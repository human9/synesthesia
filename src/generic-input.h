#ifndef GENERIC_BACKEND_H
#define GENERIC_BACKEND_H
#include "synesthesia-app.h"

gpointer generic_input(gpointer data);
void generic_disconnect();
pcmframe *generic_getbuffer(int *count, int *clear);
int generic_connection_active();

#endif
