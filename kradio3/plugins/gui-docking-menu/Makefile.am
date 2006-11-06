SUBDIRS = po .

INCLUDES = -I$(top_builddir)/kradio3/src/libkradio-gui $(all_includes)
METASOURCES = AUTO

libkradio_LTLIBRARIES =  libdocking-menu.la
libdocking_menu_la_SOURCES = docking-configuration.cpp docking.cpp
libdocking_menu_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)

noinst_HEADERS = docking-configuration.h docking.h


#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o po/kradio-gui-docking-menu.pot

messages: rc.cpp
	$(EXTRACTRC) *.rc *.ui >> rc.cpp
	$(XGETTEXT) rc.cpp *.h *.cpp -o po/kradio-gui-docking-menu.pot
