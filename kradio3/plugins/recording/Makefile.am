libdir = $(prefix)/lib/kradio/plugins
INCLUDES = $(all_includes)
METASOURCES = AUTO
lib_LTLIBRARIES =  librecording.la
librecording_la_LDFLAGS = -module -avoid-version $(all_libraries)
librecording_la_SOURCES = recording-configuration.cpp \
	recording-configuration-ui.ui recording.cpp recording-config.cpp reccfg_interfaces.cpp encoder.cpp \
	recording-datamonitor.cpp recording-monitor.cpp
noinst_HEADERS = recording-configuration.h recording.h recording-config.h \
	reccfg_interfaces.h encoder.h soundstreamevent.h recording-datamonitor.h \
	recording-monitor.h
