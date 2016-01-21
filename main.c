#include "synesthesia.h" 

int main(int argc, char *argv[])
{
	g_print("Synesthesia version %s\nCompiled %s %s\n", VERSION,  __DATE__, __TIME__);

	fftw_init();
	gtk_init(&argc, &argv);
	
	GtkApplication *app = gtk_application_new("com.coptinet.synesthesia",
		G_APPLICATION_FLAGS_NONE);

	gulong sig_id;
	g_signal_connect(app, "activate", G_CALLBACK(activate), &sig_id);

	return g_application_run (G_APPLICATION(app), argc, argv);
}

