INCLUDES = $(all_includes)
METASOURCES = AUTO

lib_LTLIBRARIES = libsoundserver.la

libsoundserver_la_SOURCES = soundserver.cpp
libsoundserver_la_LDFLAGS = -module -avoid-version $(all_libraries)
libsoundserver_la_LIBADD = ../../src/libkradio/libkradio.la

noinst_HEADERS = soundserver.h
libdir = $(prefix)/lib/kradio/plugins
