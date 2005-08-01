SUBDIRS = . 
# libkradio interfaces libkradio-gui radio-stations
METASOURCES = AUTO
INCLUDES = $(all_includes) -Iinterfaces -Ilibkradio -Ilibkradio-gui -Iradio-stations
KDE_CXXFLAGS="-UQT_NO_ASCII_CAST"
KDE_ICON = AUTO

bin_PROGRAMS = kradio

kradio_SOURCES = main.cpp \
	interfaces/errorlog-interfaces.cpp \
	interfaces/radiodevice_interfaces.cpp \
	interfaces/radiodevicepool_interfaces.cpp \
	interfaces/radio_interfaces.cpp \
	interfaces/soundstreamclient_interfaces.cpp \
	interfaces/stationselection_interfaces.cpp \
	interfaces/timecontrol_interfaces.cpp \
	libkradio/alarm.cpp \
	libkradio/debug-profiler.cpp \
	libkradio/fileringbuffer.cpp \
	libkradio/frequencyseekhelper.cpp \
	libkradio/kradioapp.cpp \
	libkradio/plugin_configuration_dialog.cpp \
	libkradio/pluginmanager-configuration.cpp \
	libkradio/pluginmanager-configuration-ui.ui \
	libkradio/pluginmanager.cpp \
	libkradio/plugins.cpp \
	libkradio/ringbuffer.cpp \
	libkradio/seekhelper.cpp \
	libkradio/soundformat.cpp \
	libkradio/soundstreamid.cpp \
	libkradio/stationlist.cpp \
	libkradio/stationlistxmlhandler.cpp \
	libkradio/utils.cpp \
	libkradio/widgetplugins.cpp \
	radio-stations/frequencyradiostation.cpp \
	radio-stations/internetradiostation.cpp \
	radio-stations/radiostation-config.cpp \
	radio-stations/radiostation.cpp \
	libkradio-gui/aboutwidget.cpp \
	libkradio-gui/radiostation-listview.cpp \
	libkradio-gui/standardscandialog.cpp \
	libkradio-gui/standardscandialog-ui.ui \
	libkradio-gui/stationselector.cpp \
	libkradio-gui/stationselector-ui.ui

kradio_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIRC) -Wl,--export-dynamic
kradio_LDADD = $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIBSOCKET)

xdg_apps_DATA = kradio.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kradio.pot
