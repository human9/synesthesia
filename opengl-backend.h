#ifndef OPENGL_BACKEND_H
#define OPENGL_BACKEND_H

#include <epoxy/gl.h>
#include "synesthesia.h"

#define OSC_NUMPOINTS 4096

gboolean glarea_render(GtkGLArea *area);
gboolean glarea_init(GtkGLArea *area);

#endif
