#include "synesthesia-app.h"
#include "synesthesia-app-window.h"
#include "preferences.h"

struct _SynesthesiaAppPrefs
{
	GtkWindow parent;
	GSettings *settings;
	GtkWidget *opacityrange;
	GtkWidget *menucheck;
	GtkWidget *cursorcheck;
};

struct _SynesthesiaAppPrefsClass
{
	GtkWindowClass parent_class;
};

typedef struct _SynesthesiaAppPrefs SynesthesiaAppPrefs;

G_DEFINE_TYPE(SynesthesiaAppPrefs, synesthesia_app_prefs, GTK_TYPE_WINDOW)

static gboolean hide_prefs(GtkWidget *window)
{
	gtk_widget_hide(window);
	return TRUE;
}

static void toggle_cursor()
{

}

static void go_normal(GtkWindow *prefwindow)
{
	//make fullscreen
	GtkWindow *window = gtk_window_get_transient_for(prefwindow);
	//GtkWindow *window = GTK_WINDOW(synesthesia_app_get_window (SYNESTHESIA_APP(app)));
	//gtk_window_fullscreen(window);
	gtk_window_unmaximize(window);
	gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(window), TRUE);

  	//gtk_widget_set_visible(GTK_WIDGET(gtk_window_get_titlebar(window)), 0);
    //gtk_window_set_position(window, GTK_WIN_POS_CENTER);

	//gtk_window_resize(window, 1280, 800);
	gtk_window_set_decorated(window, TRUE);
	//gtk_widget_set_visible(GTK_WIDGET(lower_box), false);
	//fullscreen_toggle =1;
	//synesthesia_app_window_set_opacity (SYNESTHESIA_APP_WINDOW(window), );

	//make gl background transparent
	//background_alpha=0;

	//make drawings semi-transparent
	//fade = 0.6;

	//doesn't seem to do anything fullscreen doesn't
	//gdk_window_set_override_redirect(gtk_widget_get_window(GTK_WIDGET(window)), TRUE);

	//try to make window impossible to touch
	
	synesthesia_app_window_set_opacity(SYNESTHESIA_APP_WINDOW(window), gtk_spin_button_get_value(GTK_SPIN_BUTTON(SYNESTHESIA_APP_PREFS(prefwindow)->opacityrange)));
	gtk_window_set_accept_focus(window, TRUE);
	gtk_window_set_focus_on_map(window, TRUE);
	gtk_widget_input_shape_combine_region(GTK_WIDGET(window), NULL);

	//ask wm to keep it above everything and on every desktop
	gtk_window_set_keep_above(window, FALSE);	
	//gtk_window_set_skip_taskbar_hint(window, TRUE);
	//gtk_window_set_skip_pager_hint(window, TRUE);
	gtk_window_unstick(window);
}

static void synesthesia_app_prefs_init (SynesthesiaAppPrefs *prefs)
{
	gtk_widget_init_template (GTK_WIDGET (prefs));
	prefs->settings = g_settings_new("com.coptinet.synesthesia");

	g_settings_bind (prefs->settings, "menus", prefs->menucheck, "active", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (prefs->settings, "opacity", prefs->opacityrange, "value", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (prefs->settings, "cursorhide", prefs->cursorcheck, "active", G_SETTINGS_BIND_DEFAULT);
}

static void synesthesia_app_prefs_class_init (SynesthesiaAppPrefsClass *klass)
{
	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass), "/ui/preferences.ui");

	gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), SynesthesiaAppPrefs, menucheck);
	gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), SynesthesiaAppPrefs, opacityrange);
	gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), SynesthesiaAppPrefs, cursorcheck);

	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), toggle_cursor);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), hide_prefs);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), go_normal);
}

SynesthesiaAppPrefs *synesthesia_app_prefs_new(SynesthesiaAppWindow *win)
{
	return g_object_new (synesthesia_app_prefs_get_type (), "transient-for", win, NULL);
}
