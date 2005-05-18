libdir = $(prefix)/lib/kradio/plugins
INCLUDES = $(all_includes)
METASOURCES = AUTO
lib_LTLIBRARIES =  liboss-sound.la
liboss_sound_la_LDFLAGS = -module $(all_libraries)
noinst_HEADERS = oss-sound.h oss-sound-configuration.h
liboss_sound_la_SOURCES = oss-sound.cpp oss-sound-configuration-ui.ui \
	oss-sound-configuration.cpp
