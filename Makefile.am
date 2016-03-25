LIBS = $(GTK3_LIBS) $(EPOXY_LIBS) $(FFTW3_LIBS) $(PORTAUDIO_LIBS) $(PULSEAUDIO_LIBS) -lm
FLAGS = -std=c99 -Wall $(CFLAGS) $(GTK3_CFLAGS) $(PORTAUDIO_CFLAGS) $(PULSEAUDIO_CFLAGS) 
VPATH = src:resources/ui:resources/shaders:resources/settings

SRC = main.c synesthesia-app.c synesthesia-app-window.c preferences.c shaders.c opengl-backend.c $(PULSE_FILE) $(PORT_FILE)
GEN = synesthesia-gresources.c
SCH = gschemas.compiled
BIN = synesthesia$(EXE)
HED = synesthesia-app.h synesthesia-app-window.h preferences.h shaders.h opengl-backend.h pulse-input.h port-input.h config.h
RES = f.glsl v.glsl logo.png menu.ui synesthesia.ui menu.ui preferences.ui shaders.ui separate.ui styles.css com.coptinet.synesthesia.gschema.xml
ALL = $(GEN) $(SRC)

gsettings_SCHEMAS = resources/settings/com.coptinet.synesthesia.gschema.xml

@GSETTINGS_RULES@
#GSETTINGS_SUCKS#

gschemas.compiled: resources/settings/com.coptinet.synesthesia.gschema.xml
	$(GLIB_COMPILE_SCHEMAS) resources/settings/ --targetdir=.

all: $(BIN)

synesthesia-gresources.c: synesthesia.gresource.xml $(RES)
	$(GLIB_COMPILE_RESOURCES) synesthesia.gresource.xml --target=$@ --sourcedir=resources --generate-source

$(BIN): $(SRC) $(HED) $(GEN) | $(SCH)
	$(CC) $^ -o $(@F) $(FLAGS) $(LIBS)

clean:
	rm -f $(GEN) $(SCH) $(BIN)
