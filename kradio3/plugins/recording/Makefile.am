SUBDIRS = po icons .

INCLUDES = $(all_includes)
METASOURCES = AUTO

libkradio_LTLIBRARIES = librecording.la
librecording_la_SOURCES = recording-configuration.cpp \
	recording-configuration-ui.ui recording.cpp recording-config.cpp reccfg_interfaces.cpp encoder.cpp \
	recording-datamonitor.cpp recording-monitor.cpp encoder_mp3.cpp encoder_ogg.cpp encoder_pcm.cpp
librecording_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)

noinst_HEADERS = recording-configuration.h recording.h recording-config.h \
		reccfg_interfaces.h encoder.h soundstreamevent.h recording-datamonitor.h \
		recording-monitor.h encoder_mp3.h encoder_ogg.h encoder_pcm.h

#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o po/kradio-recording.pot

messages: rc.cpp
	$(EXTRACTRC) *.rc *.ui >> rc.cpp
	$(XGETTEXT) rc.cpp *.h *.cpp -o po/kradio-recording.pot
