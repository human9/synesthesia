#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <gtk/gtk.h>
#include "synesthesia-app-window.h"

#define SYNESTHESIA_APP_PREFS_TYPE (synesthesia_app_prefs_get_type ())
#define SYNESTHESIA_APP_PREFS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), SYNESTHESIA_APP_PREFS_TYPE, SynesthesiaAppPrefs))

typedef struct _SynesthesiaAppPrefs SynesthesiaAppPrefs;
typedef struct _SynesthesiaAppPrefsClass SynesthesiaAppPrefsClass;

GType synesthesia_app_prefs_get_type (void);
GSettings *get_settings(SynesthesiaAppPrefs *self);

SynesthesiaAppPrefs *synesthesia_app_prefs_new (SynesthesiaAppWindow *win);

#endif
