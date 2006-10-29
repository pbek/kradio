SUBDIRS = interfaces radio-stations libkradio libkradio-gui .

INCLUDES = -I$(top_srcdir)/kradio3/src/interfaces \
           -I$(top_srcdir)/kradio3/src/libkradio \
           -I$(top_srcdir)/kradio3/src/libkradio-gui \
           -I$(top_srcdir)/kradio3/src/radio-stations \
           -I$(top_builddir)/kradio3/src/interfaces \
           -I$(top_builddir)/kradio3/src/libkradio \
           -I$(top_builddir)/kradio3/src/libkradio-gui \
           -I$(top_builddir)/kradio3/src/radio-stations \
           $(all_includes)

METASOURCES = AUTO

KDE_CXXFLAGS="-UQT_NO_ASCII_CAST"

#KDE_ICON = AUTO

bin_PROGRAMS = kradio

kradio_SOURCES = main.cpp
#               interfaces/errorlog-interfaces.cpp \
#               interfaces/radiodevice_interfaces.cpp \
#               interfaces/radiodevicepool_interfaces.cpp \
#               interfaces/radio_interfaces.cpp \
#               interfaces/soundstreamclient_interfaces.cpp \
#               interfaces/stationselection_interfaces.cpp \
#               interfaces/timecontrol_interfaces.cpp \
#               libkradio/alarm.cpp \
#               libkradio/debug-profiler.cpp \
#               libkradio/fileringbuffer.cpp \
#               libkradio/frequencyseekhelper.cpp \
#               libkradio/kradioapp.cpp \
#               libkradio/multibuffer.cpp \
#               libkradio/plugin_configuration_dialog.cpp \
#               libkradio/pluginmanager-configuration.cpp \
#               libkradio/pluginmanager-configuration-ui.ui \
#               libkradio/pluginmanager.cpp \
#               libkradio/plugins.cpp \
#               libkradio/ringbuffer.cpp \
#               libkradio/seekhelper.cpp \
#               libkradio/soundformat.cpp \
#               libkradio/soundstreamid.cpp \
#               libkradio/stationlist.cpp \
#               libkradio/stationlistxmlhandler.cpp \
#               libkradio/utils.cpp \
#               libkradio/widgetplugins.cpp \
#               radio-stations/frequencyradiostation.cpp \
#               radio-stations/internetradiostation.cpp \
#               radio-stations/radiostation-config.cpp \
#               radio-stations/radiostation.cpp \
#               libkradio-gui/aboutwidget.cpp \
#               libkradio-gui/radiostation-listview.cpp \
#               libkradio-gui/standardscandialog.cpp \
#               libkradio-gui/standardscandialog-ui.ui \
#               libkradio-gui/station-drag-object.cpp \
#               libkradio-gui/stationselector.cpp \
#               libkradio-gui/stationselector-ui.ui
               
kradio_LDADD = $(top_builddir)/kradio3/src/libkradio/libkradio.la \
	$(top_builddir)/kradio3/src/libkradio-gui/libkradio-gui.la $(top_builddir)/kradio3/src/interfaces/libkradio-interfaces.la \
	$(top_builddir)/kradio3/src/radio-stations/libkradio-stations.la -lkio -lkdecore -lkdeui
	
#	$(top_builddir)/kradio3/src/interfaces/errorlog-interfaces.lo \
#	$(top_builddir)/kradio3/src/interfaces/radiodevice_interfaces.lo $(top_builddir)/kradio3/src/interfaces/radiodevicepool_interfaces.lo \
#	$(top_builddir)/kradio3/src/interfaces/radio_interfaces.lo $(top_builddir)/kradio3/src/interfaces/soundstreamclient_interfaces.lo \
#	$(top_builddir)/kradio3/src/interfaces/stationselection_interfaces.lo $(top_builddir)/kradio3/src/interfaces/timecontrol_interfaces.lo \
#	$(top_builddir)/kradio3/src/libkradio/alarm.lo $(top_builddir)/kradio3/src/libkradio/debug-profiler.lo \
#	$(top_builddir)/kradio3/src/libkradio/fileringbuffer.lo $(top_builddir)/kradio3/src/libkradio/frequencyseekhelper.lo \
#	$(top_builddir)/kradio3/src/libkradio/kradioapp.lo $(top_builddir)/kradio3/src/libkradio/multibuffer.lo \
#	$(top_builddir)/kradio3/src/libkradio/plugin_configuration_dialog.lo $(top_builddir)/kradio3/src/libkradio/pluginmanager-configuration.lo \
#	$(top_builddir)/kradio3/src/libkradio/pluginmanager-configuration-ui.lo $(top_builddir)/kradio3/src/libkradio/pluginmanager.lo \
#	$(top_builddir)/kradio3/src/libkradio/plugins.lo $(top_builddir)/kradio3/src/libkradio/ringbuffer.lo \
#	$(top_builddir)/kradio3/src/libkradio/seekhelper.lo $(top_builddir)/kradio3/src/libkradio/soundformat.lo \
#	$(top_builddir)/kradio3/src/libkradio/soundstreamid.lo $(top_builddir)/kradio3/src/libkradio/stationlist.lo \
#	$(top_builddir)/kradio3/src/libkradio/stationlistxmlhandler.lo $(top_builddir)/kradio3/src/libkradio/utils.lo \
#	$(top_builddir)/kradio3/src/libkradio/widgetplugins.lo $(top_builddir)/kradio3/src/radio-stations/frequencyradiostation.lo \
#	$(top_builddir)/kradio3/src/radio-stations/internetradiostation.lo $(top_builddir)/kradio3/src/radio-stations/radiostation-config.lo \
#	$(top_builddir)/kradio3/src/radio-stations/radiostation.lo $(top_builddir)/kradio3/src/libkradio-gui/aboutwidget.lo \
#	$(top_builddir)/kradio3/src/libkradio-gui/radiostation-listview.lo $(top_builddir)/kradio3/src/libkradio-gui/standardscandialog.lo \
#	$(top_builddir)/kradio3/src/libkradio-gui/standardscandialog-ui.lo $(top_builddir)/kradio3/src/libkradio-gui/station-drag-object.lo \
#	$(top_builddir)/kradio3/src/libkradio-gui/stationselector.lo $(top_builddir)/kradio3/src/libkradio-gui/stationselector-ui.lo\

kradio_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIRC) -Wl,--export-dynamic

xdg_apps_DATA = kradio.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kradio.pot
