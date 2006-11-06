SUBDIRS = po .

INCLUDES = -I$(top_builddir)/kradio3/src/libkradio-gui -I$(top_srcdir)/kradio3 $(all_includes)
METASOURCES = AUTO

libkradio_LTLIBRARIES = libradio.la
libradio_la_SOURCES = radio-configuration.cpp radio-configuration-ui.ui \
                      radio.cpp
libradio_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)

noinst_HEADERS =  radio-configuration.h radio.h

#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o po/kradio-radio.pot

messages: rc.cpp
	$(EXTRACTRC) *.rc *.ui >> rc.cpp
	$(XGETTEXT) rc.cpp *.h *.cpp -o po/kradio-radio.pot
