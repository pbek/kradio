INCLUDES = $(all_includes)
METASOURCES = AUTO

libkradio_LTLIBRARIES = librecording.la
librecording_la_SOURCES = recording-configuration.cpp \
                          recording-configuration-ui.ui recording.cpp \
                          recording-config.cpp reccfg_interfaces.cpp \
                          encoder.cpp recording-datamonitor.cpp \
                          recording-monitor.cpp
librecording_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)

noinst_HEADERS = recording-configuration.h recording.h recording-config.h \
	reccfg_interfaces.h encoder.h soundstreamevent.h recording-datamonitor.h \
	recording-monitor.h
