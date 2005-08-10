INCLUDES = $(all_includes)
METASOURCES = AUTO

libkradio_LTLIBRARIES = libsoundserver.la

libsoundserver_la_SOURCES = soundserver.cpp
libsoundserver_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)

noinst_HEADERS = soundserver.h
