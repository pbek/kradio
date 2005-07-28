libdir = $(prefix)/lib/kradio/plugins
INCLUDES = $(all_includes)
METASOURCES = AUTO

lib_LTLIBRARIES =  libalsa-sound.la
libalsa_sound_la_LDFLAGS = -module -avoid-version $(all_libraries)
noinst_HEADERS = alsa-sound.h alsa-sound-configuration.h
libalsa_sound_la_SOURCES = alsa-sound.cpp alsa-sound-configuration-ui.ui \
	alsa-sound-configuration.cpp
libalsa_sound_la_LIBADD = ../../src/libkradio/libkradio.la -lasound
