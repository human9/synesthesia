#include "opengl-backend.h"

// GL programs and vbos
GLuint program, vbo_left, vbo_right, gl_vao;
GLint attr_osc, uni_pos; 

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

gboolean gen_program()
{
	// compile built in shaders	
	GBytes *source;
	GLuint vs, fs;
	source = g_resources_lookup_data("/shaders/v.glsl", 0, NULL);
	if ((vs = compile_shader(g_bytes_get_data(source, NULL),
		GL_VERTEX_SHADER)) == 0)
		return false;
	g_bytes_unref(source);
	source = g_resources_lookup_data("/shaders/f.glsl", 0, NULL);
	if ((fs = compile_shader(g_bytes_get_data(source, NULL),
		GL_FRAGMENT_SHADER)) == 0)
		return false;
	g_bytes_unref(source);
	
	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	
	GLint link_ok = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok)
	{
		printf("Program linking failed!");
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

gboolean glarea_init(GtkGLArea *area)
{
	gtk_gl_area_make_current(GTK_GL_AREA(area));
	if (gtk_gl_area_get_error(GTK_GL_AREA(area)) != NULL)
	{
		GError *error = gtk_gl_area_get_error(GTK_GL_AREA(area));
		g_print("Couldn't init GLArea: %s\n", error->message);
		return false;
	}
	gtk_gl_area_set_has_alpha(area, true);
	int maj, min;
	gdk_gl_context_get_version(gtk_gl_area_get_context(area), &maj, &min);
	g_print("Using OpenGL version %d.%d\n", maj, min);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	struct point {
		GLfloat x;
		GLfloat y;
	};
	struct point osc_left[OSC_NUMPOINTS];
	struct point osc_right[OSC_NUMPOINTS];

	for (int i = 0; i < OSC_NUMPOINTS; i++)
	{
		osc_left[i].x = ((float)i-OSC_NUMPOINTS/2.0)/(OSC_NUMPOINTS/2.0);
		osc_left[i].y = 0;
		osc_right[i].x = ((float)i-OSC_NUMPOINTS/2.0)/(OSC_NUMPOINTS/2.0);
		osc_right[i].y = 0;
	}

	glGenBuffers(1, &vbo_left);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_left);
	glBufferData(GL_ARRAY_BUFFER, sizeof(osc_left),
		osc_left, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &vbo_right);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_right);
	glBufferData(GL_ARRAY_BUFFER, sizeof(osc_right),
		osc_right, GL_DYNAMIC_DRAW);
	
	if (!gen_program())
		return false;

	attr_osc = get_attrib(program, "osc"); 
	uni_pos = get_uniform(program, "y_pos");

	glGenVertexArrays(1, &gl_vao);
	glBindVertexArray(gl_vao);

	return true;
}

gboolean glarea_render(GtkGLArea *area)
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(program);
	glBindVertexArray(gl_vao);
	glEnableVertexAttribArray(attr_osc);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_left);

	// Left channel
	glUniform1f(uni_pos, (float)-0.5);
	glVertexAttribPointer(
		attr_osc,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
	);
	glDrawArrays(GL_LINE_STRIP, 0, OSC_NUMPOINTS);
	
	// Right channel
	glUniform1f(uni_pos, (float)0.5);
	glVertexAttribPointer(
		attr_osc,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
	);
	glDrawArrays(GL_LINE_STRIP, 0, OSC_NUMPOINTS);

	glDisableVertexAttribArray(attr_osc);
	
	return true;
}

/*
static void oscilloscope(GtkWidget *widget)
{
	//int arraySize = 4096;
	//float left_buffer[arraySize];
	//float right_buffer[arraySize];


	//GLuint left_vbo, right_vbo;
}

gboolean repainter(GtkWidget *widget)
{
	return true;
}
*/
