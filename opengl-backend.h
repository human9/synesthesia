#ifndef OPENGL_BACKEND_H
#define OPENGL_BACKEND_H

#include <epoxy/gl.h>

GLuint compile_shader(const char* src, GLenum type);
GLboolean gen_program(GLuint *program, const char* frag);
GLint get_attrib(GLuint program, const char *name);
GLint get_uniform(GLuint program, const char *name);

#endif
