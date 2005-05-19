libdir = $(prefix)/lib/kradio/plugins
INCLUDES = -I$(top_builddir)/kradio3/src/libkradio-gui -I$(top_srcdir)/kradio3 $(all_includes)
METASOURCES = AUTO
lib_LTLIBRARIES =  libradio.la
libradio_la_LDFLAGS = -module $(all_libraries)
noinst_HEADERS =  radio-configuration.h radio.h
libradio_la_SOURCES =  radio-configuration.cpp radio-configuration-ui.ui radio.cpp
