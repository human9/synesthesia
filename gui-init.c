#include <string.h>
#include "synesthesia.h"
#include "gui-init.h"

#define ACTION_PARAMETERS GSimpleAction *action, GVariant *variant, gpointer app

float opacity = 1;
GtkWidget *preferences; // preferences dialog
const char *device_name;
GAction *old_action = NULL;
GThread *input_thread = NULL;
static int (*connection_active_ptr)(void) = NULL;
static void (*disconnect_ptr)(void) = NULL;
static gpointer (*input_ptr)(gpointer data) = NULL;
pcmframe *(*getbuffer_ptr)(int *count, int *clear) = NULL;

// function prototypes, mostly action handlers
static void opacity_change(GtkScale *scale);
static void input_swap(ACTION_PARAMETERS);
static void about(ACTION_PARAMETERS);
static void fullscreen_mode();
static void quit_app(ACTION_PARAMETERS);
static void quit_app_callback(GtkApplicationWindow *window);
static void check_alpha(GtkWidget *widget);
static void preferences_window(ACTION_PARAMETERS);
static void toggle_menubar(ACTION_PARAMETERS);
static void refresh_action(ACTION_PARAMETERS);
static void state_change(ACTION_PARAMETERS);
static void hide_prefs(ACTION_PARAMETERS);

static GActionEntry app_entries[] =
{
    { "quit", quit_app, NULL, NULL, NULL, },
    { "about", about, NULL, NULL, NULL, },
    { "fullscreen", fullscreen_mode, NULL, NULL, NULL, },
    { "preferences", preferences_window, NULL, NULL, NULL, },
    { "menubar", toggle_menubar, NULL, NULL, NULL, },
    { "refresh", refresh_action, NULL, NULL, NULL, },
#ifdef HAVE_PULSE
	{  "pulse", NULL, "s", "\"noinput\"", input_swap, },
#endif
#ifdef HAVE_PORTAUDIO
	{  "port", NULL, "s", "\"noinput\"", input_swap, },
#endif
	{  "mode", NULL, "s", "\"oscilloscope\"", state_change, }
};

void gui_init(GtkApplication *app)
{
	// get the gui layout and signals from resource
    GtkBuilder *builder = gtk_builder_new_from_resource(
        "/ui/synesthesia.ui");
	gtk_builder_add_callback_symbol(builder, "glarea_init", G_CALLBACK(glarea_init));
	gtk_builder_add_callback_symbol(builder, "glarea_render", G_CALLBACK(glarea_render));
	gtk_builder_add_callback_symbol(builder, "quit_app", G_CALLBACK(quit_app_callback));
    gtk_builder_connect_signals(builder, NULL);

	GtkBuilder *prefs = gtk_builder_new_from_resource("/ui/preferences.ui");
	gtk_builder_add_callback_symbol(prefs, "opacity_change", G_CALLBACK(opacity_change));
	gtk_builder_add_callback_symbol(prefs, "hide_prefs", G_CALLBACK(hide_prefs));
    gtk_builder_connect_signals(prefs, NULL);
	preferences = GTK_WIDGET(gtk_builder_get_object(prefs, "Preferences"));
	GSettings *settings = g_settings_new("com.coptinet.synesthesia");
	g_settings_bind (settings, "transparency", gtk_builder_get_object(prefs, "simple_scale"), "value",  G_SETTINGS_BIND_DEFAULT);
	GtkCheckButton *chkbutton = GTK_CHECK_BUTTON(gtk_builder_get_object(prefs, "menucheck"));
	g_settings_bind (settings, "menus", chkbutton, "active", G_SETTINGS_BIND_DEFAULT);

	opacity = gtk_range_get_value(GTK_RANGE(gtk_builder_get_object(prefs, "Transparency")));
	gtk_gl_area_set_required_version(GTK_GL_AREA(gtk_builder_get_object(
		builder, "glarea")), 3, 2);

    // get css from resource
	GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(provider, "/ui/styles.css");
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider), -1);

	// connect menu shortcuts
	g_action_map_add_action_entries (G_ACTION_MAP (app),
		app_entries, G_N_ELEMENTS (app_entries), app);

	// make the builder window the app window
    GtkWidget *window = GTK_WIDGET (gtk_builder_get_object (
        builder, "SynesthesiaWindow"));
	check_alpha(window);
	gtk_window_set_application(GTK_WINDOW(window), app);

	// set up menubar
	gtk_application_set_menubar(app, G_MENU_MODEL (
		gtk_builder_get_object(builder, "menubar")));

	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkbutton)))
        gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(window), false);
	
	// prefer dark theme
    g_object_set(gtk_settings_get_default(),
        "gtk-application-prefer-dark-theme", TRUE, NULL);

	// default icon
	gtk_window_set_default_icon (gdk_pixbuf_new_from_resource(
		"/ui/logo.png", NULL));


	gtk_widget_show_all(window);

}

static void input_swap(ACTION_PARAMETERS)
{
	GVariant *noinput = g_variant_new("s", "\"noinput\"");
	if (old_action != NULL)
	{
		g_simple_action_set_state(G_SIMPLE_ACTION(old_action), noinput);
	}

	device_name = g_variant_get_string(variant, NULL);
	const gchar *api_name = g_action_get_name(G_ACTION(action));
	
	if (connection_active_ptr == NULL)
	{
		#ifdef HAVE_PULSE
		if (strcmp(api_name, "pulse") == 0)
		{
			connection_active_ptr = pulse_connection_active;
			disconnect_ptr = pulse_disconnect;
			input_ptr = pulse_input;
			getbuffer_ptr = pulse_getbuffer;
		}
		#endif
		#ifdef HAVE_PORTAUDIO
		if (strcmp(api_name, "port") == 0)
		{
			connection_active_ptr = port_connection_active;
			disconnect_ptr = port_disconnect;
			input_ptr = port_input;
			getbuffer_ptr = port_getbuffer;
		}
		#endif

	}

	g_print("Connecting to %s\n", device_name);

	if(connection_active_ptr())
	{
		disconnect_ptr();
		if (input_thread != NULL)
			g_thread_join(input_thread);
	}
	
	#ifdef HAVE_PULSE
	if (strcmp(api_name, "pulse") == 0)
	{
		connection_active_ptr = pulse_connection_active;
		disconnect_ptr = pulse_disconnect;
		input_ptr = pulse_input;
		getbuffer_ptr = pulse_getbuffer;
		input_thread = g_thread_new("input_thread", input_ptr, (void *)device_name);
	}
	#endif
	#ifdef HAVE_PORTAUDIO
	if (strcmp(api_name, "port") == 0)
	{
		connection_active_ptr = port_connection_active;
		disconnect_ptr = port_disconnect;
		input_ptr = port_input;
		getbuffer_ptr = port_getbuffer;
		input_ptr((void*)device_name);
		input_thread = NULL;
	}
	#endif
	
	g_simple_action_set_state(action, variant);	
	old_action = G_ACTION(action);
}

static void toggle_menubar(ACTION_PARAMETERS)
{
	GtkApplicationWindow *appwindow = GTK_APPLICATION_WINDOW(gtk_application_get_active_window(app));
	if (gtk_application_window_get_show_menubar(appwindow))
		gtk_application_window_set_show_menubar(appwindow, false);
	else
		gtk_application_window_set_show_menubar(appwindow, true);
}

static void preferences_window(ACTION_PARAMETERS)
{
	if (!gtk_window_is_active(GTK_WINDOW(preferences)))
	{
		gtk_window_present(GTK_WINDOW(preferences));
	}
	gtk_widget_show_all(preferences);
}

static void hide_prefs(ACTION_PARAMETERS)
{
	gtk_widget_hide(preferences);
}

static void fullscreen_mode()
{

}

static void refresh_action(ACTION_PARAMETERS)
{
	refresh_devices(app);
}

static void state_change(ACTION_PARAMETERS)
{

}

static void opacity_change(GtkScale *scale)
{
	opacity = gtk_range_get_value(GTK_RANGE(scale));
}

static void check_alpha(GtkWidget *widget)
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

static void about(ACTION_PARAMETERS)
{
	char version_string[200];
	sprintf(version_string, "Version %s, compiled %s %s with %s", VERSION, __DATE__, __TIME__, COMPILED_WITH);
	gtk_show_about_dialog(gtk_application_get_active_window(app),
		"program-name", PACKAGE_NAME,
		"copyright", "© 2016 John Robert Salamon",
		"license-type", GTK_LICENSE_GPL_3_0, "version", version_string,
		"comments", "Synesthesia is a general purpose audio visualiser.", NULL);
}

static void quit_app_callback(GtkApplicationWindow *window)
{
	GtkApplication *app = gtk_window_get_application(GTK_WINDOW(window));
	#ifdef HAVE_PORTAUDIO
	if (disconnect_ptr != NULL)
		disconnect_ptr();
	#endif

	g_application_quit(G_APPLICATION(app));
	
}

static void quit_app(ACTION_PARAMETERS)
{
	#ifdef HAVE_PORTAUDIO
	if (disconnect_ptr != NULL)
		disconnect_ptr();
	#endif

	g_application_quit(G_APPLICATION(app));
}
