libdir = $(prefix)/lib/kradio/plugins
INCLUDES = -I$(top_builddir)/kradio3/src/libkradio-gui $(all_includes)
METASOURCES = AUTO
lib_LTLIBRARIES =  libquickbar.la
libquickbar_la_LDFLAGS = -module $(all_libraries)
noinst_HEADERS =  buttonflowlayout.h quickbar-configuration.h quickbar.h
libquickbar_la_SOURCES = buttonflowlayout.cpp quickbar.cpp \
	quickbar-configuration.cpp
