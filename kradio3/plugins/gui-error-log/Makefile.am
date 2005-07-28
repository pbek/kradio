libdir = $(prefix)/lib/kradio/plugins
INCLUDES = $(all_includes)
METASOURCES = AUTO

lib_LTLIBRARIES = liberror-log.la
liberror_log_la_SOURCES = errorlog.cpp
liberror_log_la_LDFLAGS = -module -avoid-version $(all_libraries)
liberror_log_la_LIBADD = ../../src/libkradio/libkradio.la

noinst_HEADERS =  errorlog.h
