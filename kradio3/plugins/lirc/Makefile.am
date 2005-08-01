libdir = $(prefix)/lib/kradio/plugins
INCLUDES = $(all_includes)
METASOURCES = AUTO

lib_LTLIBRARIES =  liblirc.la
liblirc_la_SOURCES = lircsupport.cpp lirc-configuration-ui.ui \
                     lirc-configuration.cpp
liblirc_la_LDFLAGS = -module -avoid-version $(all_libraries)
#liblirc_la_LDADD = ../../src/libkradio/libkradio.la

noinst_HEADERS = lircsupport.h lirc-configuration.h
