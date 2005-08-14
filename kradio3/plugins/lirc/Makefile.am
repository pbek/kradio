INCLUDES = $(all_includes)
METASOURCES = AUTO

libkradio_LTLIBRARIES = liblirc.la
liblirc_la_SOURCES = lircsupport.cpp lirc-configuration-ui.ui \
	lirc-configuration.cpp listviewitem_lirc.cpp
liblirc_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)
liblirc_la_LIBADD = $(LIB_LIRC)

noinst_HEADERS = lircsupport.h lirc-configuration.h listviewitem_lirc.h
