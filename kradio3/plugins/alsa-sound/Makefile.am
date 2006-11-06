SUBDIRS = po icons .

INCLUDES = $(all_includes)
METASOURCES = AUTO

libkradio_LTLIBRARIES = libalsa-sound.la
libalsa_sound_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)
noinst_HEADERS = alsa-sound.h alsa-sound-configuration.h alsa-mixer-element.h \
		alsa-config-mixer-setting.h
libalsa_sound_la_SOURCES = alsa-sound.cpp alsa-sound-configuration-ui.ui \
				alsa-sound-configuration.cpp alsa-mixer-element-ui.ui alsa-mixer-element.cpp \
		alsa-config-mixer-setting.cpp
libalsa_sound_la_LIBADD = -lasound

#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o po/kradio-alsa-sound.pot

messages: rc.cpp
	$(EXTRACTRC) *.rc *.ui >> rc.cpp
	$(XGETTEXT) rc.cpp *.h *.cpp -o po/kradio-alsa-sound.pot
