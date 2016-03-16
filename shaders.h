#ifndef SHADERS_H
#define SHADERS_H

#include <gtk/gtk.h>
#include "synesthesia-app-window.h"

#define SYNESTHESIA_APP_SHADERS_TYPE (synesthesia_app_shaders_get_type ())
#define SYNESTHESIA_APP_SHADERS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SYNESTHESIA_APP_SHADERS_TYPE, SynesthesiaAppShaders))

typedef struct _SynesthesiaAppShaders SynesthesiaAppShaders;
typedef struct _SynesthesiaAppShadersClass SynesthesiaAppShadersClass;

GType synesthesia_app_shaders_get_type (void);

SynesthesiaAppShaders *synesthesia_app_shaders_new (SynesthesiaAppWindow *win);

#endif
