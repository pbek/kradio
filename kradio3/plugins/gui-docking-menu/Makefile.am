libdir = $(prefix)/lib/kradio/plugins
INCLUDES = -I$(top_builddir)/kradio3/src/libkradio-gui $(all_includes)
METASOURCES = AUTO

lib_LTLIBRARIES =  libdocking-menu.la
libdocking_menu_la_SOURCES = docking-configuration.cpp docking.cpp
libdocking_menu_la_LDFLAGS = -module -avoid-version $(all_libraries)
libdocking_menu_la_LIBADD = ../../src/libkradio/libkradio.la

noinst_HEADERS = docking-configuration.h docking.h


