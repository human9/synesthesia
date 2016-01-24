#ifndef OPENGL_BACKEND_H
#define OPENGL_BACKEND_H

#include <epoxy/gl.h>
#include "synesthesia.h"

gboolean glarea_render(GtkGLArea *area);
gboolean glarea_init(GtkGLArea *area);

#endif
