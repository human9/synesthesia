#include <string.h>
#include <pulse/simple.h>
#include "synesthesia.h"
#include "pulse-input.h"
#include <string.h>

pcmframe temp_buffer[OSC_NUMPOINTS];
pcmframe export_buffer[OSC_NUMPOINTS];

int chunks = 0;
int clearbuff = 0;

void state_callback(pa_context *c, void *data)
{
	pa_context_state_t state = pa_context_get_state(c);
	int *ready = data;

	switch(state)
	{
	default:
		break;
	case PA_CONTEXT_TERMINATED:
		*ready = 2;
		break;
	case PA_CONTEXT_READY:
		*ready = 1;
		break;
	}
}

void source_callback(pa_context *c, const pa_source_info *l, int eol, void *data)
{
	pa_devicelist_t *pa_devicelist = data;
	pa_context_get_protocol_version(c);
	if (eol > 0)
		return;
	
	for (int ctr = 0; ctr < 16; ctr++)
	{
		if (!pa_devicelist[ctr].initialized)
		{
			strncpy(pa_devicelist[ctr].name, l->name, 511);
			strncpy(pa_devicelist[ctr].description, l->description, 255);
			pa_devicelist[ctr].index = l->index;
			pa_devicelist[ctr].initialized = 1;
			break;
		}
	}
}

int get_pulse_devices(pa_devicelist_t **sources)
{
	int state = 0;
	int ready = 0;

	pa_mainloop *loop = pa_mainloop_new();
	pa_mainloop_api *api = pa_mainloop_get_api(loop);
	pa_operation *op;
	pa_context *context = pa_context_new(api, "test");
	pa_context_connect(context, NULL, 0, NULL);
	pa_context_set_state_callback(context, state_callback, &ready);

	*sources = malloc(sizeof(pa_devicelist_t)*16);
	memset(*sources, 0 , sizeof(pa_devicelist_t) * 16);

	while(1)
	{
		if (ready == 0)
		{
			pa_mainloop_iterate(loop, 1, NULL);
			continue;
		}
		if (ready == 2)
		{
			pa_context_disconnect(context);
			pa_context_unref(context);
			pa_mainloop_free(loop);
			return -1;
		}
		switch (state)
		{
			case 0:
				op = pa_context_get_source_info_list(context,
					source_callback, *sources);
				state++;
			case 1:
				if (pa_operation_get_state(op) == PA_OPERATION_DONE)
				{
					pa_operation_unref(op);
					pa_context_disconnect(context);
					pa_context_unref(context);
					pa_mainloop_free(loop);
					return 0;
				}
				break;
			return -1;
		}
		pa_mainloop_iterate(loop, 1, NULL);
	}
}

int connected = 0;

gpointer pulse_input(gpointer data)
{
	pa_simple *s;
	pa_sample_spec ss;
	ss.format = PA_SAMPLE_S16NE;
	ss.channels = 2;
	ss.rate = 44100;
	pa_buffer_attr attr;
	attr.maxlength = (uint32_t)-1;
	attr.tlength = (uint32_t)-1;
	attr.fragsize = (uint32_t)1024;
	s = pa_simple_new(NULL,
		"Synesthesia",
		PA_STREAM_RECORD,
		(char *)data,
		"Audio input for visualisation",
		&ss,
		NULL,
		&attr,
		NULL
	);

	connected = 1;

	int error;
	pcmframe pa_buffer[BUFFSIZE*4];
	while (connected)
	{
		// our pcm is 2 channels, each channel = short int
		if (pa_simple_read(s, pa_buffer, BUFFSIZE*sizeof(short)*2, &error) < 0)
			g_print("PulseAudio error: %s\n", pa_strerror(error));
		
		memcpy(temp_buffer + chunks*BUFFSIZE, pa_buffer, BUFFSIZE*sizeof(short)*2);
		
		if (chunks * BUFFSIZE < OSC_NUMPOINTS - BUFFSIZE)
			chunks++;
		else
		{
			chunks = 0;
			clearbuff = 1;
		}
	}

	pa_simple_free(s);

	return 0;
}

int connection_active()
{
	return connected;
}

void disconnect()
{
	connected = 0;
}

pcmframe * getbuffer(int *count, int *clear)
{
	if (connected)
	{
		*count = chunks;
		*clear = clearbuff;
		memcpy(export_buffer, temp_buffer, OSC_NUMPOINTS*sizeof(short)*2);
		return export_buffer;
	}
	else
		return NULL;
}
