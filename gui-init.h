#ifndef GUI_INIT_H
#define GUI_INIT_H

#define ACTION_PARAMETERS GSimpleAction *action, GVariant *variant, gpointer app

#include <gtk/gtk.h>

void gui_init(GtkApplication *app, gulong *sig_id);

#endif
