#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "port-input.h" 

static pcmframe temp_buffer[OSC_NUMPOINTS];
static pcmframe export_buffer[OSC_NUMPOINTS];
static PaStream *stream;

static int chunks = 0;
static int clearbuff = 0;
static int connected = 0;

static int PaCallback(const void *input,
	void *output,
	unsigned long frameCount,
	const PaStreamCallbackTimeInfo *timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *data)
{
	pcmframe *pcm = (pcmframe*)input;

	memcpy(temp_buffer+chunks*BUFFSIZE, pcm, BUFFSIZE*4);
	if (chunks * BUFFSIZE < OSC_NUMPOINTS - BUFFSIZE)
		chunks++;
	else
	{
		chunks = 0;
		clearbuff = 1;
	}

	return 0;
}

static void ErrHandle(PaError error)
{
	if (error != paNoError)
	{
		printf("PortAudio: %s\n", Pa_GetErrorText(error));
		// should probably notify user
	}
}

int port_init(void)
{
	int err;
	ErrHandle(err = Pa_Initialize());
	return 0;
}

int port_kill(void)
{
	int err;
	ErrHandle(err = Pa_Terminate());
	return 0;
}

int get_port_devices(const PaDeviceInfo **deviceList[])
{
	int numDevices = Pa_GetDeviceCount();
	int a = 0;
	
	if (numDevices < 1)
	{
		port_kill();
		return 0;
	}
	else
	{
		/* get whether the device is usable for recording */
		const PaDeviceInfo **infolist = malloc(sizeof(PaDeviceInfo*) * numDevices);
		const PaDeviceInfo *temp;
		for (int i = 0; i < numDevices; i++)
		{
			temp = Pa_GetDeviceInfo(i);
			if (temp->maxInputChannels > 0)
			{
				infolist[a++] = Pa_GetDeviceInfo(i);
			}
		}
		*deviceList = infolist;
	}
	return a;
}
		
gpointer port_input(gpointer data)
{	
	int index = 0;
	int numDevices = Pa_GetDeviceCount();
	const PaDeviceInfo *temp;
	for (int i = 0; i < numDevices; i++)
	{
		temp = Pa_GetDeviceInfo(i);
		if (strcmp(temp->name, (char*)data) == 0)
		{
			index = i;
			break;
		}
	}

	PaStreamParameters params;
	params.channelCount = 2;
	params.device = index;
	params.hostApiSpecificStreamInfo = NULL;
	params.sampleFormat = paInt16;
	params.suggestedLatency = Pa_GetDeviceInfo(index)->defaultLowInputLatency;
	
	
	PaError err;
	
	ErrHandle (err = Pa_OpenStream( &stream, &params, NULL, 44100, 
		BUFFSIZE, paNoFlag, PaCallback, NULL));
		
	connected = 1;
	chunks = 0;
	clearbuff = 0;
	ErrHandle (err = Pa_StartStream(stream));

	return 0;
}

int port_connection_active()
{
	return connected;
}

void port_disconnect()
{
	PaError err;
	ErrHandle (err = Pa_StopStream(stream));
	ErrHandle (err = Pa_CloseStream(stream));
	connected = 0;
}

pcmframe *port_getbuffer(int *count, int *clear)
{
	if (connected)
	{
		*count = chunks;
		*clear = clearbuff;
		memcpy(export_buffer, temp_buffer, OSC_NUMPOINTS*4);
		return export_buffer;
	}
	else
		return NULL;
}
