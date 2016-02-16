#ifndef SYNESTHESIA_H 
#define SYNESTHESIA_H

#include "config.h"

#include "gui-init.h" 
#include "opengl-backend.h"

typedef struct
{
	short l;
	short r;
}
pcmframe;

#define BUFFSIZE 32
#define OSC_NUMPOINTS 2048 // Must be a multiple of BUFFSIZE!!!

#ifdef HAVE_PULSE
#include "pulse-input.h"
#endif

#ifdef HAVE_PORTAUDIO
#include "port-input.h"
#endif

void activate(GtkApplication *app, gulong *sig_id);
void fftw_init();
void refresh_devices(gpointer app);
#endif
