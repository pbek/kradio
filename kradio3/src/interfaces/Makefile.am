INCLUDES = $(all_includes)
METASOURCES = AUTO
noinst_LTLIBRARIES = libkradio-interfaces.la
libkradio_interfaces_la_LDFLAGS = $(all_libraries) -Wl,--export-dynamic
libkradio_interfaces_la_SOURCES = errorlog-interfaces.cpp \
                                  radiodevice_interfaces.cpp \
                                  radiodevicepool_interfaces.cpp \
                                  radio_interfaces.cpp \
                                  stationselection_interfaces.cpp \
                                  timecontrol_interfaces.cpp \
                                  soundstreamclient_interfaces.cpp
noinst_HEADERS = errorlog-interfaces.h interfaces.h radiodevice_interfaces.h \
                 radiodevicepool_interfaces.h radio_interfaces.h \
                 stationselection_interfaces.h \
                 timecontrol_interfaces.h soundstreamclient_interfaces.h
