#ifndef SYNESTHESIA_APP_H
#define SYNESTHESIA_APP_H

#include <gtk/gtk.h>
#include "../config.h"

G_BEGIN_DECLS

typedef struct
{
	short l;
	short r;
}
pcmframe;

typedef struct
{
    int (*connection_active_ptr)(void);
    void (*disconnect_ptr)(void);
    gpointer (*input_ptr)(gpointer data);
    pcmframe *(*getbuffer_ptr)(int *count, int *clear);
}
snd_ptrs;

#define BUFFSIZE 32
#define OSC_NUMPOINTS 4096

#ifdef HAVE_PULSE
#include "pulse-input.h"
#endif

#ifdef HAVE_PORTAUDIO
#include "port-input.h"
#endif

#define SYNESTHESIA_TYPE_APP (synesthesia_app_get_type ())

G_DECLARE_FINAL_TYPE (SynesthesiaApp, synesthesia_app, SYNESTHESIA, APP, GtkApplication)

gpointer synesthesia_app_get_ptrs (SynesthesiaApp *self);
GtkWidget *synesthesia_app_get_window (SynesthesiaApp *self);
GtkWidget *synesthesia_app_get_prefs (SynesthesiaApp *self);
GtkWidget *synesthesia_app_get_shaders (SynesthesiaApp *self);

GtkApplication *synesthesia_app_new (void);

G_END_DECLS

#endif
