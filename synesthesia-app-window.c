// TODO: make a SPEC_NUMPOINTS 4096
// REALLYTODO: Make both NUMPOINTS into configurable options1


#include <math.h>
#include "synesthesia-app-window.h"
#include <complex.h>
#include <fftw3.h>
#include "preferences.h"

typedef struct
{
	GLfloat x;
	GLfloat y;
}
point;

static gboolean (*glarea_render)(SynesthesiaAppWindow *window);
static gboolean (*glarea_repaint)(GtkWidget *widget, GdkFrameClock *mr_clock, gpointer data);

struct _SynesthesiaAppWindow
{
	GtkApplicationWindow parent_window;

	/* Settings and Properties (and friends) */
	GSettings *settings;
	gfloat *opacity_ptr;
	gfloat zero;
	gfloat opacity;
	guint timeout;
	gboolean timeout_exists, menuhide, cursorhide;
	gint isfullscreen, spectype;
	gint hasinit;
	
	GtkWindow *separate;
	GtkEventBox *main_box;
	
	/* OpenGL Thingamawhatzits */
	GtkGLArea *glarea;
	GLuint program, vbo_left, vbo_right, gl_vao;
	GLint attr_osc, uni_pos, uni_len;

	/* Audio Buffers and Buffer Accessories */
	pcmframe *pcm;
	pcmframe *(*getbuffer_ptr)(int *count, int *clear);
	gint count, clear;
	point *osc_left;
	point *osc_right;

	/* FFTW Complicated Things */ 
	fftw_complex *in, *out;
	fftw_plan p;
};

struct _SynesthesiaAppWindowClass
{
	GtkApplicationWindowClass parent_class;
};

G_DEFINE_TYPE (SynesthesiaAppWindow, synesthesia_app_window, GTK_TYPE_APPLICATION_WINDOW)

static gboolean render_callback(SynesthesiaAppWindow *window)
{
	glarea_render(window);
}

static gboolean repaint_callback(GtkWidget *widget, GdkFrameClock *mr_clock, gpointer data)
{
	glarea_repaint(widget, mr_clock, data);
}

GtkGLArea* synesthesia_app_window_get_glarea(SynesthesiaAppWindow *self)
{
	return self->glarea;
}

GLuint* synesthesia_app_window_get_program(SynesthesiaAppWindow *self)
{
	return &self->program;
}

gint synesthesia_app_window_get_isfullscreen (SynesthesiaAppWindow *self)
{
	return self->isfullscreen;
}

void synesthesia_app_window_set_isfullscreen (SynesthesiaAppWindow *self,
												gint isfullscreen)
{
	if (self->isfullscreen != isfullscreen)
	{
		self->isfullscreen = isfullscreen;
	}
}


gfloat synesthesia_app_window_get_opacity (SynesthesiaAppWindow *self)
{
	return self->opacity;
}

void synesthesia_app_window_set_opacity (SynesthesiaAppWindow *self,
	gfloat opacity)
{
	if (self->opacity != opacity)
	{
		self->opacity = opacity;
	}
}

static gboolean synesthesia_app_window_get_menuhide (SynesthesiaAppWindow *self)
{
	return self->menuhide;
}

static void synesthesia_app_window_set_menuhide (SynesthesiaAppWindow *self,
	gboolean menuhide)
{
	if (self->menuhide != menuhide)
	{
		self->menuhide = menuhide;
	}
}

static gboolean synesthesia_app_window_get_cursorhide (SynesthesiaAppWindow *self)
{
	return self->cursorhide;
}

static void synesthesia_app_window_set_cursorhide (SynesthesiaAppWindow *self,
	gboolean cursorhide)
{
	if (self->cursorhide != cursorhide)
	{
		self->cursorhide = cursorhide;
	}
}

static int start_timeout(SynesthesiaAppWindow *window)
{
	if (window->isfullscreen)
	{
		GdkCursor *blankcursor = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_BLANK_CURSOR);
		if (window->cursorhide)
			gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(window)), blankcursor);
	}
	window->timeout_exists = FALSE;
	return 0;
}

static int mouse_over(SynesthesiaAppWindow *window)
{
	if (window->timeout_exists)
		g_source_remove(window->timeout);
	window->timeout = g_timeout_add_seconds(1, (GSourceFunc)start_timeout, window);
	window->timeout_exists = TRUE;

	gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(window)), NULL);

	return 0;
}

static gboolean window_state_event(SynesthesiaAppWindow *window, GdkEventWindowState *event)
{
	if (event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN)
	{
		if (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN)
		{
			synesthesia_app_window_set_isfullscreen (window, 1);
			GdkCursor *blankcursor = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_BLANK_CURSOR);
			if (window->cursorhide)
				gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(window)), blankcursor);
			g_signal_handlers_unblock_by_func(window, mouse_over, window);
		}
		else
		{
			synesthesia_app_window_set_isfullscreen (window, 0);
			gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(window)), NULL);
			g_signal_handlers_block_by_func(window, mouse_over, window);
		}
	}	
}

static void quit_app(GtkApplicationWindow *window)
{
	GtkApplication *app = gtk_window_get_application(GTK_WINDOW(window));
	g_application_quit(G_APPLICATION(app));
}

gboolean button_event (GtkWidget *widget, GdkEventButton *event)
{
	//toggle_spectype(SYNESTHESIA_APP_WINDOW(gtk_widget_get_toplevel(widget)));	
	return TRUE;
}

enum {
	PROP_0,
	ISFULLSCREEN,
	OPACITY,
	MENUHIDE,
	CURSORHIDE,
	LAST_PROP
};

static GParamSpec *properties [LAST_PROP];

static void synesthesia_app_window_get_property (GObject *object, guint prop_id, GValue *value,
	GParamSpec *pspec)
{
	SynesthesiaAppWindow *self = (SynesthesiaAppWindow *)object;
	switch (prop_id)
	{
		case ISFULLSCREEN:
			g_value_set_int(value, synesthesia_app_window_get_isfullscreen (self));
			break;
		case OPACITY:
			g_value_set_double(value, synesthesia_app_window_get_opacity (self));
			break;
		case MENUHIDE:
			g_value_set_boolean(value, synesthesia_app_window_get_menuhide (self));
			break;
		case CURSORHIDE:
			g_value_set_boolean(value, synesthesia_app_window_get_cursorhide (self));
			break;
	}
}

static void synesthesia_app_window_set_property (GObject *object, guint prop_id, const GValue *value,
	GParamSpec *pspec)
{
	SynesthesiaAppWindow *self = (SynesthesiaAppWindow *)object;
	switch (prop_id)
	{
		case ISFULLSCREEN:
		;
			gint state = g_value_get_int(value);
			synesthesia_app_window_set_isfullscreen (self, state); 
			break;
		case OPACITY:
		;
			gfloat opacity = g_value_get_double(value);
			synesthesia_app_window_set_opacity (self, opacity);
			break;
		case MENUHIDE:
		;
			gboolean menu = g_value_get_boolean(value);
			synesthesia_app_window_set_menuhide (self, menu);
			break;
		case CURSORHIDE:
		;
			gboolean cursor = g_value_get_boolean(value);
			synesthesia_app_window_set_cursorhide (self, cursor);
			break;
	}
}

gboolean glarea_init(SynesthesiaAppWindow *window)
{
	window->spectype = 1;
	window->osc_left = g_new0(point, OSC_NUMPOINTS);
	window->osc_right = g_new0(point, OSC_NUMPOINTS);
	if (!window->hasinit)
		gtk_widget_add_tick_callback(GTK_WIDGET(window->glarea), (GtkTickCallback)repaint_callback, window, NULL);
	
	gtk_gl_area_make_current(GTK_GL_AREA(window->glarea));
	if (gtk_gl_area_get_error(GTK_GL_AREA(window->glarea)) != NULL)
	{
		GError *error = gtk_gl_area_get_error(window->glarea);
		g_print("Couldn't init GLArea: %s\n", error->message);
		return false;
	}
	gtk_gl_area_set_has_alpha(window->glarea, true);
	int maj, min;
	gdk_gl_context_get_version(gtk_gl_area_get_context(window->glarea), &maj, &min);
	g_print("Using OpenGL version %d.%d\n", maj, min);

	glEnable(GL_BLEND);
	glEnable(GL_PROGRAM_POINT_SIZE);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glGenBuffers(1, &(window->vbo_left));
	glBindBuffer(GL_ARRAY_BUFFER, window->vbo_left);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point) * OSC_NUMPOINTS,
		window->osc_left, GL_STREAM_DRAW);

	glGenBuffers(1, &(window->vbo_right));
	glBindBuffer(GL_ARRAY_BUFFER, window->vbo_right);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point) * OSC_NUMPOINTS,
		window->osc_right, GL_STREAM_DRAW);
	
	if (!gen_program(&(window->program), NULL))
		return false;

	window->attr_osc = get_attrib(window->program, "osc"); 
	window->uni_pos = get_uniform(window->program, "y_pos");
	window->uni_len = get_uniform(window->program, "len");

	glGenVertexArrays(1, &(window->gl_vao));
	glBindVertexArray(window->gl_vao);


	if (1)
	{
		for (int i = 0; i < OSC_NUMPOINTS; i++)
		{
			window->osc_left[i].x = i;
			window->osc_left[i].y = 0;
			window->osc_right[i].x = i; 
			window->osc_right[i].y = 0;
		}
		window->hasinit = 1;
	}

	return true;
}

static gboolean glarea_render_spectrum(SynesthesiaAppWindow *window)
{

	glClearColor(0, 0, 0, *window->opacity_ptr);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(window->program);
	glBindVertexArray(window->gl_vao);
	glEnableVertexAttribArray(window->attr_osc);
	glUniform1f(window->uni_len, OSC_NUMPOINTS / 2.0);

	// Left channel
	glBindBuffer(GL_ARRAY_BUFFER, window->vbo_left);
	glUniform1f(window->uni_pos, 0);
	glVertexAttribPointer(
		window->attr_osc,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
	);
	glDrawArrays(GL_POINTS, 0, OSC_NUMPOINTS);
	
	// Right channel
	glBindBuffer(GL_ARRAY_BUFFER, window->vbo_right);
	glUniform1f(window->uni_pos, 0);
	glVertexAttribPointer(
		window->attr_osc,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		//(GLvoid *)(sizeof(point)*(OSC_NUMPOINTS/2))
		0
	);
	glDrawArrays(GL_POINTS, 0, OSC_NUMPOINTS);

	glDisableVertexAttribArray(window->attr_osc);
	
	return true;
}

static gboolean glarea_render_oscilloscope(SynesthesiaAppWindow *window)
{

	glClearColor(0, 0, 0, *window->opacity_ptr);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(window->program);
	glBindVertexArray(window->gl_vao);
	glEnableVertexAttribArray(window->attr_osc);
	glUniform1f(window->uni_len, OSC_NUMPOINTS / 2.0);

	// Left channel
	glBindBuffer(GL_ARRAY_BUFFER, window->vbo_left);
	glUniform1f(window->uni_pos, 0.5);
	glVertexAttribPointer(
		window->attr_osc,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
	);
	glDrawArrays(GL_LINE_STRIP, 0, OSC_NUMPOINTS);
	
	// Right channel
	glBindBuffer(GL_ARRAY_BUFFER, window->vbo_right);
	glUniform1f(window->uni_pos, -0.5);
	glVertexAttribPointer(
		window->attr_osc,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
	);
	glDrawArrays(GL_LINE_STRIP, 0, OSC_NUMPOINTS);

	glDisableVertexAttribArray(window->attr_osc);
	
	return true;
}

void window_set_buffer_ptr(SynesthesiaAppWindow *window, gpointer getbuffer)
{
	window->getbuffer_ptr = getbuffer;
}

static void update_buffers(SynesthesiaAppWindow *window)
{
	int i; // counts the current position in the output buffers
	
	if (window->clear)
	// the input buffer size has cleared the output size
	{
		for (i = 0; i < OSC_NUMPOINTS - window->count * BUFFSIZE; i++)
		{
			window->osc_left[i].y = window->pcm[i + window->count * BUFFSIZE].l;
			window->osc_right[i].y = window->pcm[i + window->count * BUFFSIZE].r;
		}
	}

	else
	// the input buffer hasn't got enough data to clear output, so shift it leftwards
	{
		for (i = 0; i < OSC_NUMPOINTS - window->count * BUFFSIZE; i++)
		{
			window->osc_left[i].y = window->osc_left[i + window->count*BUFFSIZE].y;
			window->osc_right[i].y = window->osc_right[i + window->count*BUFFSIZE].y;
		}
	}

	for (int a = 0; i < OSC_NUMPOINTS; i++ && a++) 
	// this fills the rest of output if input has wrapped around (count != 0)
	{
		window->osc_left[i].y = window->pcm[i + window->count * BUFFSIZE].l;
		window->osc_right[i].y = window->pcm[i + window->count * BUFFSIZE].r;
	}
	
}

static double scaler(double input, int times)
{
    for (int i=0; i < times; i++)
    {
        input = log(input + 1.0) / log(2.0);
    }
    return input;
}

static gboolean glarea_repaint_spectrum(GtkWidget *widget, GdkFrameClock *mr_clock, gpointer data)
{
	SynesthesiaAppWindow *window = SYNESTHESIA_APP_WINDOW(data);

	if (window->getbuffer_ptr != NULL && (window->pcm = window->getbuffer_ptr(&window->count, &window->clear)) != NULL)
	{
		update_buffers(window);
	
		switch(window->spectype)
		{
			case 0:
				break;
			case 1:
				for (int i = 0; i < OSC_NUMPOINTS; i++) 
				{
					window->osc_left[i].y /= 2e5;
					window->osc_right[i].y /= 2e5; 
				}
		}
		for (int i = 0; i < OSC_NUMPOINTS; i++)
		{
			window->in[i] = (double complex)window->osc_left[i].y;
		}

		fftw_execute(window->p);
		
		int spacing = OSC_NUMPOINTS / 512;
		int freq = 0;

		for (int i = 0; i < OSC_NUMPOINTS; i++)
        {
            switch(window->spectype)
            {
            case 0:

                window->osc_left[i].y = (float)
				( (log(10 * creal(window->out[i]*conj(window->out[i])))/log(1000) ) -1);
                break;

            case 1:

                if (i % spacing == 0)
                {
					
                    double value = (creal(window->out[freq]*conj(window->out[freq])));
                    window->osc_left[i].y = (float)scaler(value, 6);
                    freq++;
                }
                else
                {
                    window->osc_left[i].y = 0;
                }
                break;
            }
        }

        for (int i = 0; i < OSC_NUMPOINTS; i++)
        {
            window->in[i] = (double complex)window->osc_right[i].y;
        }	

		fftw_execute(window->p);

		freq = 0;

		for (int i = 0; i < OSC_NUMPOINTS; i++)
        {
            switch(window->spectype)
            {
            case 0:
                window->osc_right[i].y = (float)
				( (log(10 * creal(window->out[i]*conj(window->out[i])))/log(1000) ) -1);
                break;
            case 1:
                if (i % spacing == 0)
                {
                    double value = (creal(window->out[freq]*conj(window->out[freq])));
                    window->osc_right[i].y = (float)scaler(value, 6) * -1;
                    freq++;
                }
                else
                {
                    window->osc_right[i].y = 0;
                }
                break;
            }

        }
		
		glBindBuffer(GL_ARRAY_BUFFER, window->vbo_left);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point) * OSC_NUMPOINTS, window->osc_left);
		
		glBindBuffer(GL_ARRAY_BUFFER, window->vbo_right);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point) * OSC_NUMPOINTS, window->osc_right);

		gtk_widget_queue_draw(widget);
	}
	return 1;
}

static gboolean glarea_repaint_oscilloscope(GtkWidget *widget, GdkFrameClock *mr_clock, gpointer data)
{
	SynesthesiaAppWindow *window = SYNESTHESIA_APP_WINDOW(data);

	if (window->getbuffer_ptr != NULL && (window->pcm = window->getbuffer_ptr(&window->count, &window->clear)) != NULL)
	{
		update_buffers(window);	
		// make it fit in the -1 to 1 range
		for (int i = 0; i < OSC_NUMPOINTS; i++) 
		{
			window->osc_left[i].y /= 32768.0; 
			window->osc_right[i].y /= 32768.0;  
		}
		
		glBindBuffer(GL_ARRAY_BUFFER, window->vbo_left);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point) * OSC_NUMPOINTS, window->osc_left);
		
		glBindBuffer(GL_ARRAY_BUFFER, window->vbo_right);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point) * OSC_NUMPOINTS, window->osc_right);

		gtk_widget_queue_draw(widget);
	}
	return 1;
}

static void force_alpha(GtkWidget *widget)
{
    GdkScreen *screen = gtk_widget_get_screen(widget);
    GdkVisual *visual = gdk_screen_get_rgba_visual(screen);

    if (!visual)
    {
        g_print("Transparency is unavailable\n");
        visual = gdk_screen_get_system_visual(screen);
    }
    else
        g_print("Transparency enabled\n");

    gtk_widget_set_visual(widget, visual);
}

void separate_window(SynesthesiaAppWindow *self, gboolean above)
{
	GtkBuilder *builder;
	builder = gtk_builder_new_from_resource("/ui/separate.ui");
	self->separate = GTK_WINDOW(gtk_builder_get_object(builder, "render-window"));
	GtkWindow *window = self->separate;
	gtk_window_set_attached_to(window, GTK_WIDGET(self));
	force_alpha(GTK_WIDGET(window));
	gtk_widget_show_all(GTK_WIDGET(window));

	gtk_container_remove(GTK_CONTAINER(self->main_box), GTK_WIDGET(self->glarea)); 
	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(self->glarea));
	//make fullscreen
	//gtk_window_fullscreen(window);
	gtk_window_maximize(window);
	//gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(window), 0);

  	//gtk_widget_set_visible(GTK_WIDGET(gtk_window_get_titlebar(window)), 0);
    //gtk_window_set_position(window, GTK_WIN_POS_CENTER);

	//gtk_window_resize(window, 1280, 800);
	gtk_window_set_decorated(window, false);
	//gtk_widget_set_visible(GTK_WIDGET(lower_box), false);
	//fullscreen_toggle =1;
	//synesthesia_app_window_set_opacity (SYNESTHESIA_APP_WINDOW(window), 0);
	self->opacity_ptr = &self->zero;
	//make gl background transparent
	//background_alpha=0;

	//make drawings semi-transparent
	//fade = 0.6;

	//doesn't seem to do anything fullscreen doesn't
	//gdk_window_set_override_redirect(gtk_widget_get_window(GTK_WIDGET(window)), TRUE);

	//try to make window impossible to touch
	gtk_window_set_accept_focus(window, FALSE);
	gtk_window_set_focus_on_map(window, FALSE);
	cairo_region_t *region = cairo_region_create();
	gtk_widget_input_shape_combine_region(GTK_WIDGET(window), region);

	//ask wm to keep it above everything and on every desktop
	gtk_window_set_keep_above(window, TRUE);	
	gtk_window_set_skip_taskbar_hint(window, TRUE);
	gtk_window_set_skip_pager_hint(window, TRUE);
	gtk_window_stick(window);
	gtk_window_present(GTK_WINDOW(self));
	g_object_unref(builder);
}

void main_window(SynesthesiaAppWindow *self)
{
	GtkWindow *window = self->separate;
	self->opacity_ptr = &self->opacity;
	gtk_container_remove(GTK_CONTAINER(self->separate), GTK_WIDGET(self->glarea)); 
	gtk_container_add(GTK_CONTAINER(self->main_box), GTK_WIDGET(self->glarea));

	gtk_widget_destroy(GTK_WIDGET(self->separate));
}


void toggle_spectype(SynesthesiaAppWindow *window)
{
	switch(window->spectype)
	{
		case 0:
			window->spectype = 1;
			break;
		case 1:
			window->spectype = 0;
			break;
	}
}

void set_oscilloscope()
{
	glarea_render = glarea_render_oscilloscope;
	glarea_repaint = glarea_repaint_oscilloscope; 
}

void set_spectrum()
{
	glarea_render = glarea_render_spectrum;
	glarea_repaint = glarea_repaint_spectrum; 
}

static void synesthesia_app_window_class_init (SynesthesiaAppWindowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	
	object_class->get_property = synesthesia_app_window_get_property;
	object_class->set_property = synesthesia_app_window_set_property;

	properties [ISFULLSCREEN] =
		g_param_spec_string ("isfullscreen", "isfullscreen", "is it fullscreen", NULL,
		(G_PARAM_READWRITE));
	
	properties [OPACITY] =
		g_param_spec_double ("opacity", "opacity", "gl context opacity", 0, 1, 1,
		(G_PARAM_READWRITE));
	
	properties [MENUHIDE] =
		g_param_spec_boolean ("menuhide", "menuhide", "hide menu on new window", FALSE,
		(G_PARAM_READWRITE));
	
	properties [CURSORHIDE] =
		g_param_spec_boolean ("cursorhide", "cursorhide", "hide cursor in fullscreen", FALSE,
		(G_PARAM_READWRITE));

	g_object_class_install_properties (object_class, LAST_PROP, properties);
	
	

	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	gtk_widget_class_set_template_from_resource (widget_class, "/ui/synesthesia.ui");

	/* hrumph */
	glarea_render = glarea_render_oscilloscope;
	glarea_repaint = glarea_repaint_oscilloscope; 

	gtk_widget_class_bind_template_child (widget_class, SynesthesiaAppWindow, glarea);
	gtk_widget_class_bind_template_child (widget_class, SynesthesiaAppWindow, main_box);

	gtk_widget_class_bind_template_callback (widget_class, window_state_event);
	gtk_widget_class_bind_template_callback (widget_class, render_callback);
	gtk_widget_class_bind_template_callback (widget_class, glarea_init);
	gtk_widget_class_bind_template_callback (widget_class, quit_app);
	gtk_widget_class_bind_template_callback (widget_class, button_event);
	gtk_widget_class_bind_template_callback (widget_class, mouse_over);
}


static void synesthesia_app_window_init (SynesthesiaAppWindow *self)
{
	gtk_widget_init_template (GTK_WIDGET (self));
	
	self->settings = g_settings_new ("com.coptinet.synesthesia");

	g_settings_bind (self->settings, "opacity", self, "opacity", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (self->settings, "menus", self, "menuhide", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (self->settings, "cursorhide", self, "cursorhide", G_SETTINGS_BIND_DEFAULT);

	g_signal_handlers_block_by_func(self, mouse_over, self);
	self->isfullscreen = -1;
	force_alpha(GTK_WIDGET(self));
	self->opacity_ptr = &self->opacity;
	self->zero = 0;
	self->in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * OSC_NUMPOINTS);
	self->out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * OSC_NUMPOINTS);
	self->p = fftw_plan_dft_1d(OSC_NUMPOINTS, self->in, self->out, FFTW_FORWARD, FFTW_MEASURE);
}

GtkWidget *synesthesia_app_window_new (SynesthesiaApp *app)
{
	return g_object_new (synesthesia_app_window_get_type (), "application", app, NULL);
}
