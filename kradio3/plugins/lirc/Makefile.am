INCLUDES = $(all_includes)
METASOURCES = AUTO

libkradio_LTLIBRARIES = liblirc.la
liblirc_la_SOURCES = lircsupport.cpp lirc-configuration-ui.ui \
                     lirc-configuration.cpp
liblirc_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)

noinst_HEADERS = lircsupport.h lirc-configuration.h
