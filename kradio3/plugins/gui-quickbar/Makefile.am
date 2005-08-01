libdir = $(prefix)/lib/kradio/plugins
INCLUDES = -I$(top_builddir)/kradio3/src/libkradio-gui $(all_includes)
METASOURCES = AUTO

lib_LTLIBRARIES =  libquickbar.la
libquickbar_la_SOURCES = buttonflowlayout.cpp quickbar.cpp \
                         quickbar-configuration.cpp
libquickbar_la_LDFLAGS = -module -avoid-version $(all_libraries)
#libquickbar_la_LIBADD = ../../src/libkradio/libkradio.la

noinst_HEADERS =  buttonflowlayout.h quickbar-configuration.h quickbar.h
