SUBDIRS = po icons .

INCLUDES = $(all_includes)
METASOURCES = AUTO

libkradio_LTLIBRARIES = libstreaming.la
libstreaming_la_SOURCES = streaming.cpp streaming-configuration-ui.ui \
	streaming-configuration.cpp streaming-job.cpp
libstreaming_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)

noinst_HEADERS = streaming.h streaming-configuration.h streaming-job.h

#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o po/kradio-streaming.pot

messages: rc.cpp
	$(EXTRACTRC) *.rc *.ui >> rc.cpp
	$(XGETTEXT) rc.cpp *.h *.cpp -o po/kradio-streaming.pot
