INCLUDES = $(all_includes)
METASOURCES = AUTO

libkradio_LTLIBRARIES = libalsa-sound.la
libalsa_sound_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)
noinst_HEADERS = alsa-sound.h alsa-sound-configuration.h
libalsa_sound_la_SOURCES = alsa-sound.cpp alsa-sound-configuration-ui.ui \
	alsa-sound-configuration.cpp
libalsa_sound_la_LIBADD = -lasound
