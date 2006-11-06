SUBDIRS = po icons .

INCLUDES = $(all_includes)
METASOURCES = AUTO

libkradio_LTLIBRARIES = libtimecontrol.la
libtimecontrol_la_SOURCES = timecontrol-configuration.cpp \
                            timecontrol-configuration-ui.ui timecontrol.cpp
libtimecontrol_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)
noinst_HEADERS =  timecontrol-configuration.h timecontrol.h

#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o po/kradio-timecontrol.pot

messages: rc.cpp
	$(EXTRACTRC) *.rc *.ui >> rc.cpp
	$(XGETTEXT) rc.cpp *.h *.cpp -o po/kradio-timecontrol.pot
