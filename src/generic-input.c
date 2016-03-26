#include <string.h>
#include <stdio.h>
#include "generic-input.h"

static pcmframe temp_buffer[OSC_NUMPOINTS];
static pcmframe export_buffer[OSC_NUMPOINTS];

static int connected = 0;
static int chunks = 0;
static int clearbuff = 0;

gpointer generic_input(gpointer data)
{
	connected = 1;
	chunks = 0;
	clearbuff = 0;

	pcmframe buff[BUFFSIZE*4];
	size_t size = BUFFSIZE*sizeof(short)*2;

	if (filename != NULL)
	{
		FILE *f = fopen(filename, "rb");
		while (connected)
		{
			// our pcm is 2 channels, each channel = short int
			fread(buff, size, 1, f);
			
			memcpy(temp_buffer + chunks*BUFFSIZE, buff, size);
			
			if (chunks * BUFFSIZE < OSC_NUMPOINTS - BUFFSIZE)
				chunks++;
			else
			{
				chunks = 0;
				clearbuff = 1;
			}
		}

		fclose(f);
	}

	return 0;
}

int generic_connection_active()
{
	return connected;
}

void generic_disconnect()
{
	connected = 0;
}

pcmframe * generic_getbuffer(int *count, int *clear)
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
