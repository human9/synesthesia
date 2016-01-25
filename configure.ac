dnf use autoreconf -i to generate configure script

AC_INIT([Synesthesia], [0.2], [jrs@coptinet.com], , [https://coptinet.com])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([autotools])

AM_INIT_AUTOMAKE
AC_PROG_CC
AC_DEFINE_UNQUOTED([COMPILED_WITH],"$CC", [Name of compiler])
AC_CHECK_PROG(HAVE_PKG_CONFIG, pkg-config)
AC_PATH_PROG(GLIB_COMPILE_RESOURCES, glib-compile-resources)
PKG_CHECK_MODULES(GTK3, gtk+-3.0 >= 3.16)
PKG_CHECK_MODULES(EPOXY, epoxy)
PKG_CHECK_MODULES(FFTW3, fftw3)
PKG_CHECK_MODULES(XML, libxml-2.0)

AC_ARG_ENABLE([pulse],
	AS_HELP_STRING([--disable-pulse], [Disable PulseAudio backend]))
AS_IF([test "x$enable_pulse" != "xno"], [
	PKG_CHECK_MODULES(PULSEAUDIO, libpulse-simple, have_pulse=true, have_pulse=false)
	if test "x${have_pulse}" = "xtrue" ; then
		AC_DEFINE([HAVE_PULSE], [1], [Use PulseAudio])
		AC_SUBST([PULSE_FILE], ["pulse-input.c"])
	fi
])

AC_ARG_ENABLE([port],
	AS_HELP_STRING([--disable-port], [Disable PortAudio backend]))
AS_IF([test "x$enable_port" != "xno"], [
	PKG_CHECK_MODULES(PORTAUDIO, portaudio-2.0, have_port=true, have_port=false)
	if test "x${have_port}" = "xtrue" ; then
		AC_DEFINE([HAVE_PORTAUDIO], [1], [Use PulseAudio])
		AC_SUBST([PORT_FILE], ["port-input.c"])
	fi
])

mingw="mingw"
if test "${CC#*$mingw}" != "$CC"; then
	AC_SUBST([EXE],[".exe"])
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
