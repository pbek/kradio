SUBDIRS = po .

INCLUDES = -I$(top_builddir)/kradio3/src $(all_includes)
METASOURCES = AUTO

libkradio_LTLIBRARIES = libquickbar.la
libquickbar_la_SOURCES = buttonflowlayout.cpp quickbar.cpp \
                         quickbar-configuration.cpp
libquickbar_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)

noinst_HEADERS =  buttonflowlayout.h quickbar-configuration.h quickbar.h

#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o po/kradio-gui-quickbar.pot

messages: rc.cpp
	$(EXTRACTRC) *.rc *.ui >> rc.cpp
	$(XGETTEXT) rc.cpp *.h *.cpp -o po/kradio-gui-quickbar.pot
