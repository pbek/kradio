SUBDIRS = po .

INCLUDES = $(all_includes)

METASOURCES = AUTO

KDE_CXXFLAGS="-UQT_NO_ASCII_CAST"

bin_PROGRAMS = kradio

kradio_SOURCES = \
               main.cpp 				\
               errorlog-interfaces.cpp 			\
               radiodevice_interfaces.cpp 		\
               radiodevicepool_interfaces.cpp 		\
               radio_interfaces.cpp 			\
               soundstreamclient_interfaces.cpp 	\
               stationselection_interfaces.cpp 		\
               timecontrol_interfaces.cpp 		\
               alarm.cpp 				\
               debug-profiler.cpp 			\
               fileringbuffer.cpp 			\
               frequencyseekhelper.cpp 			\
               kradioapp.cpp 				\
               multibuffer.cpp 				\
               plugin_configuration_dialog.cpp 		\
               pluginmanager-configuration.cpp 		\
               pluginmanager-configuration-ui.ui 	\
               pluginmanager.cpp 			\
               plugins.cpp 				\
               ringbuffer.cpp 				\
               seekhelper.cpp 				\
               soundformat.cpp 				\
               soundstreamid.cpp 			\
               stationlist.cpp 				\
               stationlistxmlhandler.cpp 		\
               utils.cpp 				\
               widgetplugins.cpp 			\
               radio-stations/frequencyradiostation.cpp	\
               radio-stations/internetradiostation.cpp 	\
               radio-stations/radiostation-config.cpp 	\
               radio-stations/radiostation.cpp 		\
               aboutwidget.cpp 				\
               radiostation-listview.cpp 		\
               standardscandialog.cpp 			\
               standardscandialog-ui.ui 		\
               station-drag-object.cpp 			\
               stationselector.cpp 			\
               stationselector-ui.ui

noinst_HEADERS = \
		aboutwidget.h			\
		alarm.h				\
		debug-profiler.h		\
		errorlog-interfaces.h		\
		fileringbuffer.h		\
		frequencyradiostation.h		\
		frequencyseekhelper.h		\
		gui_list_helper.h		\
		interfaces.h			\
		internetradiostation.h		\
		kradioapp.h			\
		kradioversion.h			\
		multibuffer.h			\
		plugin_configuration_dialog.h	\
		pluginmanager-configuration.h	\
		pluginmanager.h			\
		plugins.h			\
		radiodevice_interfaces.h	\
		radiodevicepool_interfaces.h	\
		radio_interfaces.h		\
		radiostation-config.h		\
		radiostation.h			\
		radiostation-listview.h		\
		ringbuffer.h			\
		seekhelper.h			\
		soundformat.h			\
		sound_metadata.h		\
		soundstreamclient_interfaces.h	\
		soundstreamid.h			\
		standardscandialog.h		\
		station-drag-object.h		\
		stationlist.h			\
		stationlistmetadata.h		\
		stationlistxmlhandler.h		\
		stationselection_interfaces.h	\
		stationselector.h		\
		timecontrol_interfaces.h	\
		utils.h				\
		widgetplugins.h

kradio_LDADD = -lkio -lkdecore -lkdeui

kradio_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIRC) -Wl,--export-dynamic

xdg_apps_DATA = kradio.desktop

messages: rc.cpp
	$(EXTRACTRC)       *.ui  *.rc >> rc.cpp
	$(XGETTEXT) rc.cpp *.cpp *.h  -o po/kradio.pot
