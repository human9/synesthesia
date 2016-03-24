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
	GtkWidget *osc_adjustment;
	GtkWidget *spec_adjustment;
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

static void synesthesia_app_prefs_init (SynesthesiaAppPrefs *prefs)
{
	gtk_widget_init_template (GTK_WIDGET (prefs));
	prefs->settings = g_settings_new("com.coptinet.synesthesia");

	g_settings_bind (prefs->settings, "menus", prefs->menucheck, "active", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (prefs->settings, "opacity", prefs->opacityrange, "value", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (prefs->settings, "oscgain", prefs->osc_adjustment, "value", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (prefs->settings, "specgain", prefs->spec_adjustment, "value", G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (prefs->settings, "cursorhide", prefs->cursorcheck, "active", G_SETTINGS_BIND_DEFAULT);
}

static void synesthesia_app_prefs_class_init (SynesthesiaAppPrefsClass *klass)
{
	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass), "/ui/preferences.ui");

	gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), SynesthesiaAppPrefs, menucheck);
	gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), SynesthesiaAppPrefs, opacityrange);
	gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), SynesthesiaAppPrefs, cursorcheck);
	gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), SynesthesiaAppPrefs, osc_adjustment);
	gtk_widget_class_bind_template_child (GTK_WIDGET_CLASS (klass), SynesthesiaAppPrefs, spec_adjustment);

	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (klass), hide_prefs);
}

SynesthesiaAppPrefs *synesthesia_app_prefs_new(SynesthesiaAppWindow *win)
{
	return g_object_new (synesthesia_app_prefs_get_type (), "transient-for", win, NULL);
}
