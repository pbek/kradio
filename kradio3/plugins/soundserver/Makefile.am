SUBDIRS = po .

INCLUDES = $(all_includes)
METASOURCES = AUTO

libkradio_LTLIBRARIES = libsoundserver.la

libsoundserver_la_SOURCES = soundserver.cpp
libsoundserver_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)

noinst_HEADERS = soundserver.h

#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o po/kradio-sound-server.pot

messages: rc.cpp
	$(EXTRACTRC) *.rc *.ui >> rc.cpp
	$(XGETTEXT) rc.cpp *.h *.cpp -o po/kradio-soundserver.pot
