libdir = $(prefix)/lib/kradio/plugins
INCLUDES = $(all_includes)
METASOURCES = AUTO

lib_LTLIBRARIES =  libv4lradio.la
libv4lradio_la_SOURCES =  v4lcfg_interfaces.cpp v4lradio-configuration.cpp \
                          v4lradio-configuration-ui.ui v4lradio.cpp
libv4lradio_la_LDFLAGS = -module -avoid-version $(all_libraries)
#libv4lradio_la_LIBADD = ../../src/libkradio/libkradio.la \
#                        ../../src/radio-stations/libkradio-stations.la

noinst_HEADERS = v4lcfg_interfaces.h v4lradio-configuration.h v4lradio.h

