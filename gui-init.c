#include "gui-init.h"
#include "synesthesia.h"

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

static void quit_app(GSimpleAction *action,
	GVariant *parameter, gpointer app)
{
	g_application_quit(G_APPLICATION(app));
}

static void about(GSimpleAction *action,
	GVariant *parameter, gpointer app)
{
	char version_string[200];
	sprintf(version_string, "Version %s, compiled %s %s with %s", VERSION, __DATE__, __TIME__, COMPILED_WITH);
	gtk_show_about_dialog(gtk_application_get_active_window(app),
		"program-name", PACKAGE_NAME,
		"copyright", "© 2016 John Robert Salamon",
		"license-type", GTK_LICENSE_GPL_3_0, "version", version_string,
		"comments", "Synesthesia is a general purpose audio visualiser.", NULL);
}
static void fullscreen_mode()
{

}

static void refresh_action(GSimpleAction *action,
	GVariant *parameter, gpointer app)
{
	refresh_devices(app);
}

static void state_change(GSimpleAction *action,
	GVariant *variant, gpointer app)
{

}

GThread *input_thread;
const char *device_name;
#ifdef HAVE_PULSE
static void pulse_input_swap(GSimpleAction *action,
	GVariant *variant, gpointer app)
{
	device_name = g_variant_get_string(variant, NULL);
	g_print("Connecting to %s\n", device_name);

	if(connection_active())
	{
		disconnect();
		g_thread_join(input_thread);
	}
	input_thread = g_thread_new("input_thread", pulse_input, (void *)device_name);

	g_simple_action_set_state(action, variant);	
}
#endif

static GActionEntry app_entries[] =
{
    { "quit", quit_app, NULL, NULL, NULL, },
    { "about", about, NULL, NULL, NULL, },
    { "fullscreen", fullscreen_mode, NULL, NULL, NULL, },
    { "refresh", refresh_action, NULL, NULL, NULL, },
#ifdef HAVE_PULSE
	{  "pulse", NULL, "s", "\"placeholder\"", pulse_input_swap, },
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
    gtk_builder_connect_signals(builder, NULL);

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

	// prefer dark theme
    g_object_set(gtk_settings_get_default(),
        "gtk-application-prefer-dark-theme", TRUE, NULL);

	// default icon
	gtk_window_set_default_icon (gdk_pixbuf_new_from_resource(
		"/ui/logo.png", NULL));


	gtk_widget_show_all(window);

}
