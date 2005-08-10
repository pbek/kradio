SUBDIRS = libkradio interfaces libkradio-gui radio-stations .

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

KDE_ICON = AUTO

bin_PROGRAMS = kradio

kradio_SOURCES = main.cpp
kradio_LDADD = interfaces/errorlog-interfaces.lo \
               interfaces/radiodevice_interfaces.lo \
               interfaces/radiodevicepool_interfaces.lo \
               interfaces/radio_interfaces.lo \
               interfaces/soundstreamclient_interfaces.lo \
               interfaces/stationselection_interfaces.lo \
               interfaces/timecontrol_interfaces.lo \
               libkradio/alarm.lo \
               libkradio/debug-profiler.lo \
               libkradio/fileringbuffer.lo \
               libkradio/frequencyseekhelper.lo \
               libkradio/kradioapp.lo \
               libkradio/plugin_configuration_dialog.lo \
               libkradio/pluginmanager-configuration.lo \
               libkradio/pluginmanager-configuration-ui.lo \
               libkradio/pluginmanager.lo \
               libkradio/plugins.lo \
               libkradio/ringbuffer.lo \
               libkradio/seekhelper.lo \
               libkradio/soundformat.lo \
               libkradio/soundstreamid.lo \
               libkradio/stationlist.lo \
               libkradio/stationlistxmlhandler.lo \
               libkradio/utils.lo \
               libkradio/widgetplugins.lo \
               radio-stations/frequencyradiostation.lo \
               radio-stations/internetradiostation.lo \
               radio-stations/radiostation-config.lo \
               radio-stations/radiostation.lo \
               libkradio-gui/aboutwidget.lo \
               libkradio-gui/radiostation-listview.lo \
               libkradio-gui/standardscandialog.lo \
               libkradio-gui/standardscandialog-ui.lo \
               libkradio-gui/stationselector.lo \
               libkradio-gui/stationselector-ui.lo \
               $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIBSOCKET)

kradio_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIRC) -Wl,--export-dynamic

xdg_apps_DATA = kradio.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kradio.pot
