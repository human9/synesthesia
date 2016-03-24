#include <gtk/gtk.h>

#include "synesthesia-app.h"

int main(int argc, char *argv[])
{
/*	
	g_print("Synesthesia version %s\n\n", VERSION);

	fftw_init();
	
	GtkApplication *app = gtk_application_new("com.coptinet.synesthesia",
		G_APPLICATION_FLAGS_NONE);

	gulong sig_id;
	g_signal_connect(app, "activate", G_CALLBACK(activate), &sig_id);
*/
	return g_application_run(G_APPLICATION(synesthesia_app_new ()), argc, argv);
}

