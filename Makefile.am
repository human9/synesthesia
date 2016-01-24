LIBS = $(GTK3_LIBS) $(EPOXY_LIBS) $(FFTW3_LIBS) $(PORTAUDIO_LIBS) $(PULSEAUDIO_LIBS) -lm
FLAGS = $(CFLAGS) $(GTK3_CFLAGS) $(PORTAUDIO_CFLAGS) $(PULSEAUDIO_CFLAGS) 

SRC = main.c gui-init.c opengl-backend.c synesthesia.c $(PULSE_FILE) $(PORT_FILE) 
GEN = synesthesia-gresources.c
BIN = synesthesia$(EXE)
HED = synesthesia.h gui-init.h opengl-backend.h pulse-input.h port-input.h
RES = resources/ui/styles.css resources/ui/synesthesia.ui resources/ui/logo.png resources/shaders/f.glsl resources/shaders/v.glsl

ALL = $(GEN) $(SRC)

all: $(BIN)

synesthesia-gresources.c: synesthesia.gresource.xml $(RES)
	$(GLIB_COMPILE_RESOURCES) synesthesia.gresource.xml --target=$@ --sourcedir=resources --generate-source

$(BIN): $(GEN) $(SRC) $(HED)
	$(CC) $(ALL) -o $(@F) $(FLAGS) $(LIBS)

clean:
	rm -f $(GEN)
	rm -f $(BIN)
