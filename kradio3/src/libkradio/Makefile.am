INCLUDES = $(all_includes)
METASOURCES = AUTO
noinst_LTLIBRARIES = libkradio.la
libkradio_la_LDFLAGS = $(all_libraries)
libkradio_la_SOURCES = plugin_configuration_dialog.cpp pluginmanager.cpp \
                       plugins.cpp seekhelper.cpp stationlistxmlhandler.cpp \
                       utils.cpp widgetplugins.cpp \
                       kradioapp.cpp frequencyseekhelper.cpp alarm.cpp \
                       stationlist.cpp ringbuffer.cpp \
                       pluginmanager-configuration-ui.ui \
                       pluginmanager-configuration.cpp fileringbuffer.cpp \
                       soundstreamid.cpp soundformat.cpp debug-profiler.cpp
noinst_HEADERS = plugin_configuration_dialog.h pluginmanager.h plugins.h \
                 seekhelper.h stationlistmetadata.h stationlistxmlhandler.h \
                 utils.h widgetplugins.h \
                 kradioapp.h frequencyseekhelper.h alarm.h stationlist.h \
                 ringbuffer.h pluginmanager-configuration.h fileringbuffer.h \
                 soundstreamid.h soundformat.h sound_metadata.h \
                 debug-profiler.h
libkradio_la_DEPENDENCIES = pluginmanager-configuration-ui.h

