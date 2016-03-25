#include <glib/gprintf.h>
#include "synesthesia-app.h"
#include "synesthesia-app-window.h"
#include "preferences.h"
#include "shaders.h"

#ifdef HAVE_PULSE
#include "pulse-input.h"
#endif

#ifdef HAVE_PORTAUDIO
#include "port-input.h"
#endif



#define ACTION_PARAMETERS GSimpleAction *action, GVariant *variant, gpointer app

/*
gboolean button_event(GtkWidget *widget, GdkEventButton *event);
gboolean window_state_event(GtkWidget *window, GdkEventWindowState *event, gpointer sig_id);
static int mouse_over(GtkWidget *window);
static void opacity_change(GtkSpinButton *spinbutton);
static void quit_app_callback(GtkApplicationWindow *window);
static void check_alpha(GtkWidget *widget);
static void toggle_cursor(GtkWidget *widget);
static void hide_prefs(ACTION_PARAMETERS);
*/

/* action functions */
static void quit_app(ACTION_PARAMETERS);
static void about(ACTION_PARAMETERS);
static void fullscreen_mode(ACTION_PARAMETERS);
static void preferences_window(ACTION_PARAMETERS);
static void shaders_window(ACTION_PARAMETERS);
static void toggle_menubar(ACTION_PARAMETERS);
static void refresh_action(ACTION_PARAMETERS);
static void input_swap(ACTION_PARAMETERS);
static void state_change(ACTION_PARAMETERS);
static void above_all(ACTION_PARAMETERS);


struct _SynesthesiaApp
{
	GtkApplication parent_app;

	GtkWidget *window;

	GtkWidget *preferences;
	GtkWidget *shaders;

	gint cursor_autohide;
	const char *device_name;
	GAction *old_action;
	GAction *vis_old_action;
	GThread *input_thread;

	snd_ptrs ptrs;
};

struct _SynesthesiaAppClass
{
	GtkApplicationClass parent_class;
};

G_DEFINE_TYPE (SynesthesiaApp, synesthesia_app, GTK_TYPE_APPLICATION)


static GActionEntry app_entries[] =
{
    { "quit", quit_app, NULL, NULL, NULL, },
    { "about", about, NULL, NULL, NULL, },
    { "fullscreen", fullscreen_mode, NULL, NULL, NULL, },
    { "above", above_all, NULL, NULL, NULL, },
    { "preferences", preferences_window, NULL, NULL, NULL, },
    { "shaders", shaders_window, NULL, NULL, NULL, },
    { "menubar", toggle_menubar, NULL, NULL, NULL, },
    { "refresh", refresh_action, NULL, NULL, NULL, },
#ifdef HAVE_PULSE
    {  "pulse", NULL, "s", "\"noinput\"",input_swap, },
#endif
#ifdef HAVE_PORTAUDIO
    {  "port", NULL, "s", "\"noinput\"", input_swap, },
#endif
    {  "mode", NULL, "s", "\"oscilloscope\"", state_change, }
};

gpointer synesthesia_app_get_ptrs (SynesthesiaApp *self)
{
	return &(self->ptrs);
}

GtkWidget *synesthesia_app_get_window (SynesthesiaApp *self)
{
	return self->window;
}

GtkWidget *synesthesia_app_get_prefs (SynesthesiaApp *self)
{
	return self->preferences;
}
GtkWidget *synesthesia_app_get_shaders (SynesthesiaApp *self)
{
	return self->shaders;
}

static void quit_app(ACTION_PARAMETERS)
{
	#ifdef HAVE_PORTAUDIO

	snd_ptrs *ptrs = synesthesia_app_get_ptrs(app);
	if (ptrs->disconnect_ptr != NULL)
		ptrs->disconnect_ptr();
	#endif

	g_application_quit(G_APPLICATION(app));
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

static void fullscreen_mode(ACTION_PARAMETERS)
{
	GtkWindow *window = GTK_WINDOW (SYNESTHESIA_APP(app)->window);	
	int state = synesthesia_app_window_get_isfullscreen (SYNESTHESIA_APP_WINDOW(window));	

	switch (state)
	{
		case -1:
			gtk_window_fullscreen(window);
			break;
		case 0:
			gtk_window_fullscreen(window);
			break;
		case 1:
			gtk_window_unfullscreen(window);
			break;
	}
}

int temp = 0;
static void above_all(ACTION_PARAMETERS)
{
	switch(temp)
	{
	case 0:
		separate_window(SYNESTHESIA_APP_WINDOW(SYNESTHESIA_APP(app)->window), TRUE);
		temp = 1;
		break;
	case 1:
		main_window(SYNESTHESIA_APP_WINDOW(SYNESTHESIA_APP(app)->window));
		temp = 0;
		break;
	}
}

static void preferences_window(ACTION_PARAMETERS)
{
	gtk_window_present (GTK_WINDOW (SYNESTHESIA_APP(app)->preferences));
	/*if (!gtk_window_is_active(GTK_WINDOW(preferences)))
	{
		gtk_window_present(GTK_WINDOW(preferences));
	}
	gtk_widget_show_all(preferences);*/
}

static void shaders_window(ACTION_PARAMETERS)
{
	gtk_window_present (GTK_WINDOW (SYNESTHESIA_APP(app)->shaders));
	/*if (!gtk_window_is_active(GTK_WINDOW(preferences)))
	{
		gtk_window_present(GTK_WINDOW(preferences));
	}
	gtk_widget_show_all(preferences);*/
}

static void toggle_menubar(ACTION_PARAMETERS)
{
	GtkApplicationWindow *appwindow = GTK_APPLICATION_WINDOW(gtk_application_get_active_window(app));
	if (gtk_application_window_get_show_menubar(appwindow))
		gtk_application_window_set_show_menubar(appwindow, FALSE);
	else
		gtk_application_window_set_show_menubar(appwindow, TRUE);
}

static void refresh_devices(gpointer app)
{
	g_print("Refreshing available devices...\n");
	int num_found = 0;
	GMenu *device_menu = g_menu_new();

	#ifdef HAVE_PULSE 

	GMenu *device = g_menu_new();
	
	pa_devicelist_t *sources;
	get_pulse_devices(&sources);
	for(int i = 0; i < 16; i++)
	{
		if (!sources[i].initialized)
			break;
		gchar action_string[526];
		g_sprintf(action_string, "app.pulse::%s", sources[i].name);
		g_menu_append(device, sources[i].description, action_string);
		num_found++;
	}
	// Insert the devices we got
	g_menu_append_submenu(device_menu, "PulseAudio", G_MENU_MODEL(device));

	#endif

	#ifdef HAVE_PORTAUDIO

	GMenu *portdevice = g_menu_new();

	const PaDeviceInfo **deviceList;
	int deviceNum = get_port_devices(&deviceList);
	for(int i = 0; i < deviceNum; i++)
	{
		gchar action_string[526];
		g_sprintf(action_string, "app.port::%s", deviceList[i]->name);
		g_menu_append(portdevice, deviceList[i]->name, action_string);
		num_found++;
	}
	// Insert the devices we got
	g_menu_append_submenu(device_menu, "PortAudio", G_MENU_MODEL(portdevice));

	#endif

	if (num_found > 0)
		g_print("%d devices found.\n", num_found);
	else
		g_print("No devices available\n");

	// Add the refresh item
	GMenu *control = g_menu_new();
	g_menu_append(control, "Refresh", "app.refresh");
	g_menu_append_section(device_menu, NULL, G_MENU_MODEL(control));
	
	// Put the new Devices menu together
	GMenuItem *newDevices = g_menu_item_new_submenu(
		"_Devices", G_MENU_MODEL(device_menu)); 	

	
	// remove original Devices menu
	GMenuModel *menubar = gtk_application_get_menubar(app);	
	g_menu_remove(G_MENU(menubar), 3);
	// insert new Devices menu
	g_menu_insert_item(G_MENU(menubar), 3, newDevices);
	
}

static void refresh_action(ACTION_PARAMETERS)
{
	refresh_devices(app);
}

static void input_swap(ACTION_PARAMETERS)
{
	SynesthesiaApp *synapp = SYNESTHESIA_APP (app);

    GVariant *noinput = g_variant_new("s", "\"noinput\"");
    if (synapp->old_action != NULL)
    {
        g_simple_action_set_state(G_SIMPLE_ACTION(synapp->old_action), noinput);
    }

    synapp->device_name = g_variant_get_string(variant, NULL);
    const gchar *api_name = g_action_get_name(G_ACTION(action));

    if (synapp->ptrs.connection_active_ptr == NULL)
    {
        #ifdef HAVE_PULSE
        if (g_strcmp0(api_name, "pulse") == 0)
        {
            synapp->ptrs.connection_active_ptr = pulse_connection_active;
            synapp->ptrs.disconnect_ptr = pulse_disconnect;
            synapp->ptrs.input_ptr = pulse_input;
            synapp->ptrs.getbuffer_ptr = pulse_getbuffer;
        }
        #endif
        #ifdef HAVE_PORTAUDIO
        if (g_strcmp0(api_name, "port") == 0)
        {
            synapp->ptrs.connection_active_ptr = port_connection_active;
            synapp->ptrs.disconnect_ptr = port_disconnect;
            synapp->ptrs.input_ptr = port_input;
            synapp->ptrs.getbuffer_ptr = port_getbuffer;
        }
        #endif

    }

    g_print("Connecting to %s\n", synapp->device_name);

    if(synapp->ptrs.connection_active_ptr())
    {
        synapp->ptrs.disconnect_ptr();
        if (synapp->input_thread != NULL)
            g_thread_join(synapp->input_thread);
    }

    #ifdef HAVE_PULSE
    if (g_strcmp0(api_name, "pulse") == 0)
    {
        synapp->ptrs.connection_active_ptr = pulse_connection_active;
        synapp->ptrs.disconnect_ptr = pulse_disconnect;
        synapp->ptrs.input_ptr = pulse_input;
        synapp->ptrs.getbuffer_ptr = pulse_getbuffer;
        synapp->input_thread = g_thread_new("input_thread", synapp->ptrs.input_ptr, (void *)synapp->device_name);
    }
    #endif
    #ifdef HAVE_PORTAUDIO
    if (g_strcmp0(api_name, "port") == 0)
    {
        synapp->ptrs.connection_active_ptr = port_connection_active;
        synapp->ptrs.disconnect_ptr = port_disconnect;
        synapp->ptrs.input_ptr = port_input;
        synapp->ptrs.getbuffer_ptr = port_getbuffer;
        synapp->ptrs.input_ptr((void*)synapp->device_name);
        synapp->input_thread = NULL;
    }
    #endif

    g_simple_action_set_state(action, variant);
    synapp->old_action = G_ACTION(action);
	window_set_buffer_ptr(SYNESTHESIA_APP_WINDOW(synapp->window), synapp->ptrs.getbuffer_ptr);
}

static void state_change(ACTION_PARAMETERS)
{
	const char *mode = g_variant_get_string(variant, NULL);
	g_print("Now in %s mode\n", mode);
	
	if (g_strcmp0(mode, "spectrum") == 0)
	{
		set_spectrum();	
	}
	else
	{
		set_oscilloscope();
	}
	
	g_simple_action_set_state(action, variant);	
	SYNESTHESIA_APP(app)->vis_old_action = G_ACTION(action);
}

static void synesthesia_app_startup (GApplication *app)
{
	GtkBuilder *builder;
	GMenuModel *menu_bar;

	#ifdef HAVE_PORTAUDIO
	port_init();
	#endif

	G_APPLICATION_CLASS (synesthesia_app_parent_class)->startup (app);

	g_action_map_add_action_entries (G_ACTION_MAP (app),
        app_entries, G_N_ELEMENTS (app_entries), app);


	GtkCssProvider *provider = gtk_css_provider_new();
	gtk_css_provider_load_from_resource(provider, "/ui/styles.css");
	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
	        GTK_STYLE_PROVIDER(provider), -1);
	
	gtk_window_set_default_icon (gdk_pixbuf_new_from_resource(
	        "/ui/logo.png", NULL));

	builder = gtk_builder_new_from_resource("/ui/menu.ui");
	menu_bar = G_MENU_MODEL (gtk_builder_get_object (builder, "menubar"));
	gtk_application_set_menubar(GTK_APPLICATION(app), menu_bar);
	
	g_object_unref (builder); 
	g_object_unref (provider);
}

static void synesthesia_app_activate (GApplication *app)
{
	SynesthesiaApp *self = SYNESTHESIA_APP (app);
	

	if (self->window == NULL)
		self->window = synesthesia_app_window_new (SYNESTHESIA_APP (app));

	gtk_window_present (GTK_WINDOW (self->window));
	
	if (self->preferences == NULL)
		self->preferences = GTK_WIDGET(synesthesia_app_prefs_new (SYNESTHESIA_APP_WINDOW (self->window)));
	if (self->shaders == NULL)
		self->shaders = GTK_WIDGET(synesthesia_app_shaders_new (SYNESTHESIA_APP_WINDOW (self->window)));

	refresh_devices(app);
	
}

static void synesthesia_app_class_init (SynesthesiaAppClass *klass)
{
	GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

	app_class->startup = synesthesia_app_startup;
	app_class->activate = synesthesia_app_activate;
}

static void synesthesia_app_init (SynesthesiaApp *self)
{
}

GtkApplication * synesthesia_app_new (void)
{
	return g_object_new (synesthesia_app_get_type (), "application-id", "com.coptinet.synesthesia", NULL);
}
