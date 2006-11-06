SUBDIRS = po icons .

INCLUDES = $(all_includes)
METASOURCES = AUTO

libkradio_LTLIBRARIES = liboss-sound.la
liboss_sound_la_SOURCES = oss-sound.cpp oss-sound-configuration-ui.ui \
                          oss-sound-configuration.cpp
liboss_sound_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)

noinst_HEADERS = oss-sound.h oss-sound-configuration.h

#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o po/kradio-oss-sound.pot

messages: rc.cpp
	$(EXTRACTRC) *.rc *.ui >> rc.cpp
	$(XGETTEXT) rc.cpp *.h *.cpp -o po/kradio-oss-sound.pot
