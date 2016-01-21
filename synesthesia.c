#include <fftw3.h>
#include "synesthesia.h"

#define ARRAYSIZE 4096

void fftw_init()
{
/*	fftw_complex *in, *out;
	fftw_plan p;

	in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * ARRAYSIZE);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * ARRAYSIZE);
    p = fftw_plan_dft_1d(ARRAYSIZE, in, out, FFTW_FORWARD, FFTW_MEASURE);
*/
}

void activate(GtkApplication *app, gulong *sig_id)
{
	gui_init(app);
	refresh_devices(app);
}

void refresh_devices(gpointer app)
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
		sprintf(action_string, "app.pulse::%s", sources[i].name);
		g_menu_append(device, sources[i].description, action_string);
		num_found++;
	}
	// Insert the devices we got
	g_menu_append_submenu(device_menu, "Pulse", G_MENU_MODEL(device));

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
	g_menu_remove(G_MENU(menubar), 2);
	// insert new Devices menu
	g_menu_insert_item(G_MENU(menubar), 2, newDevices);
	
}
