#include <string.h>
#include "synesthesia-app.h"
#include "synesthesia-app-window.h"
#include "shaders.h"

struct _SynesthesiaAppShaders
{
	GtkWindow parent;
	GtkTextView *vertview;
	GtkTextView *fragview;
	GtkTextBuffer *vbuffer;
	GtkTextBuffer *fbuffer;
	GtkWidget *infobar;
	GtkWidget *message;
};

struct _SynesthesiaAppShadersClass
{
	GtkWindowClass parent_class;
};

typedef struct _SynesthesiaAppShaders SynesthesiaAppShaders;

G_DEFINE_TYPE(SynesthesiaAppShaders, synesthesia_app_shaders, GTK_TYPE_WINDOW)

static gboolean hide_shaders(GtkWidget *window)
{
	gtk_widget_hide(window);
	return TRUE;
}

static gboolean compile_buffer(SynesthesiaAppShaders *shaders)
{

	GtkWindow *window = gtk_window_get_transient_for(GTK_WINDOW(shaders));
	GLuint *program = synesthesia_app_window_get_program(SYNESTHESIA_APP_WINDOW(window));
	GtkTextIter sti, eni;
	gtk_text_buffer_get_bounds(shaders->vbuffer, &sti, &eni);
	const char *vert = gtk_text_buffer_get_text(shaders->vbuffer, &sti, &eni, FALSE);
	gtk_text_buffer_get_bounds(shaders->fbuffer, &sti, &eni);
	const char *frag = gtk_text_buffer_get_text(shaders->fbuffer, &sti, &eni, FALSE);
	if (gen_program(program, vert, frag))
	{
		gtk_label_set_text(GTK_LABEL(shaders->message), "Compilation Done!");
	}
	else
		gtk_label_set_text(GTK_LABEL(shaders->message), "GLSL compile error!");

	gtk_widget_show(shaders->infobar);
	return TRUE;
}

static void loadbuffer(const char *resource, GtkTextBuffer *buffer, GtkTextView *view)
{
	GBytes *src = g_resources_lookup_data(resource, 0, NULL);
	const char* srcstr = g_bytes_get_data(src, NULL);
	gtk_text_buffer_set_text(buffer, srcstr, strlen(srcstr));
	gtk_text_view_set_buffer(view, buffer);
}

static void synesthesia_app_shaders_init (SynesthesiaAppShaders *shaders)
{
	gtk_widget_init_template (GTK_WIDGET (shaders));
	shaders->vbuffer = gtk_text_buffer_new(NULL);
	shaders->fbuffer = gtk_text_buffer_new(NULL);
	loadbuffer("/shaders/v.glsl", shaders->vbuffer, shaders->vertview);
	loadbuffer("/shaders/f.glsl", shaders->fbuffer, shaders->fragview);
	
	shaders->message = gtk_label_new("You should not be reading this");
	gtk_widget_show(shaders->message);
	GtkWidget *content = gtk_info_bar_get_content_area(GTK_INFO_BAR(shaders->infobar));
	gtk_container_add (GTK_CONTAINER (content), shaders->message);
	gtk_info_bar_add_button (GTK_INFO_BAR(shaders->infobar), "_OK", GTK_RESPONSE_OK);
	g_signal_connect(shaders->infobar, "response", G_CALLBACK(gtk_widget_hide), NULL);
}

static void synesthesia_app_shaders_class_init (SynesthesiaAppShadersClass *klass)
{
	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass), "/ui/shaders.ui");
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), hide_shaders);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), compile_buffer);
	gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), SynesthesiaAppShaders, vertview);
	gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), SynesthesiaAppShaders, fragview);
	gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), SynesthesiaAppShaders, infobar);
}

SynesthesiaAppShaders *synesthesia_app_shaders_new(SynesthesiaAppWindow *win)
{
	return g_object_new (synesthesia_app_shaders_get_type (), "transient-for", win, NULL);
}
