#include "opengl-backend.h"
#include "synesthesia-app.h"

GLuint compile_shader(const char* src, GLenum type)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
	GLint compile_ok = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_ok);
	
	if (compile_ok == GL_FALSE)
	{
		g_print("%s shader failed to compile!\n", 
			type == GL_VERTEX_SHADER ? "Vertex" : "Fragment");
		GLint logsize = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);

		GLchar log[logsize];
		glGetShaderInfoLog(shader, logsize, NULL, log);
		g_print("*** GLSL COMPILE LOG ***\n%s\n*** END LOG ***\n", log);

		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

GLboolean gen_program(GLuint *program, const char* vert, const char* frag)
{
	// compile built in shaders	
	GBytes *source;
	GLuint vs, fs;
	const char *vsrc;
	/* I think there's undefined behaviour here somewhere but I can't find it. */
	if (vert == NULL)
	{
		source = g_resources_lookup_data("/shaders/v.glsl", 0, NULL);
		vsrc = g_bytes_get_data(source, NULL);
	}
	else
	{
		vsrc = vert;
	}
	if ((vs = compile_shader(vsrc, GL_VERTEX_SHADER)) == 0)
		return false;
	g_bytes_unref(source);
	
	const char* fsrc;
	if (frag == NULL)
	{
		source = g_resources_lookup_data("/shaders/f.glsl", 0, NULL);
		fsrc = g_bytes_get_data(source, NULL);
	}
	else
	{
		fsrc = frag;
	}
	if ((fs = compile_shader(fsrc, GL_FRAGMENT_SHADER)) == 0)
		return false;
	
	g_bytes_unref(source);
	
	*program = glCreateProgram();
	glAttachShader(*program, vs);
	glAttachShader(*program, fs);
	glLinkProgram(*program);
	
	GLint link_ok = GL_FALSE;
	glGetProgramiv(*program, GL_LINK_STATUS, &link_ok);
	if (!link_ok)
	{
		printf("Program linking failed!\n");
		return false;
	}

	return true;
}

GLint get_attrib(GLuint program, const char *name)
{
	GLint attribute = glGetAttribLocation(program, name);
	if (attribute == -1)
		g_print("%s failed to bind\n", name);
	return attribute;
}

GLint get_uniform(GLuint program, const char *name)
{
	GLint uniform = glGetUniformLocation(program, name);
	if (uniform == -1)
		g_print("%s failed to bind\n", name);
	return uniform;
}

