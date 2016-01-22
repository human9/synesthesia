#ifndef SYNESTHESIA_H 
#define SYNESTHESIA_H

#include "config.h"

#include "gui-init.h" 
#include "opengl-backend.h"

#ifdef HAVE_PULSE
#include "pulse-input.h"
#endif

void activate(GtkApplication *app, gulong *sig_id);
void fftw_init();
void refresh_devices(gpointer app);
void input_swap(GVariant *variant, gpointer app);
#endif
