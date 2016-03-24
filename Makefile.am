LIBS = $(GTK3_LIBS) $(EPOXY_LIBS) $(FFTW3_LIBS) $(PORTAUDIO_LIBS) $(PULSEAUDIO_LIBS) -lm
FLAGS = -std=c99 $(CFLAGS) $(GTK3_CFLAGS) $(PORTAUDIO_CFLAGS) $(PULSEAUDIO_CFLAGS) 

SRC = main.c synesthesia-app.c synesthesia-app-window.c preferences.c shaders.c opengl-backend.c $(PULSE_FILE) $(PORT_FILE)
GEN = synesthesia-gresources.c
SCH = gschemas.compiled
BIN = synesthesia$(EXE)
HED = synesthesia-app.h synesthesia-app-window.h preferences.h shaders.h opengl-backend.h pulse-input.h port-input.h config.h
RES = resources/ui/* resources/shaders/* resources/settings/*
ALL = $(GEN) $(SRC)

gsettings_SCHEMAS = resources/settings/com.coptinet.synesthesia.gschema.xml

@GSETTINGS_RULES@
#GSETTINGS_SUCKS#

gschemas.compiled: resources/settings/com.coptinet.synesthesia.gschema.xml
	$(GLIB_COMPILE_SCHEMAS) resources/settings/ --targetdir=.

all: $(BIN)

synesthesia-gresources.c: synesthesia.gresource.xml $(RES)
	$(GLIB_COMPILE_RESOURCES) synesthesia.gresource.xml --target=$@ --sourcedir=resources --generate-source

$(BIN): $(GEN) $(SRC) $(HED) $(SCH)
	$(CC) $(ALL) -o $(@F) $(FLAGS) $(LIBS)

clean:
	rm -f $(GEN) $(SCH) $(BIN)
