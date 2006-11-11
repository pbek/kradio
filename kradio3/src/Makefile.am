SUBDIRS = po .

INCLUDES =	-I$(top_srcdir)/kradio3/src/include \
		-I$(top_builddir)/kradio3/src/include \
		$(all_includes)

METASOURCES = AUTO

META_INCLUDES = $(srcdir)/include

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
               frequencyradiostation.cpp	\
               internetradiostation.cpp 	\
               radiostation-config.cpp 	\
               radiostation.cpp 		\
               aboutwidget.cpp 				\
               radiostation-listview.cpp 		\
               standardscandialog.cpp 			\
               standardscandialog-ui.ui 		\
               station-drag-object.cpp 			\
               stationselector.cpp 			\
               stationselector-ui.ui

noinst_HEADERS = \
		include/aboutwidget.h			\
		include/alarm.h				\
		include/debug-profiler.h		\
		include/errorlog-interfaces.h		\
		include/fileringbuffer.h		\
		include/frequencyradiostation.h		\
		include/frequencyseekhelper.h		\
		include/gui_list_helper.h		\
		include/interfaces.h			\
		include/internetradiostation.h		\
		include/kradioapp.h			\
		include/kradioversion.h			\
		include/multibuffer.h			\
		include/plugin_configuration_dialog.h	\
		include/pluginmanager-configuration.h	\
		include/pluginmanager.h			\
		include/plugins.h			\
		include/radiodevice_interfaces.h	\
		include/radiodevicepool_interfaces.h	\
		include/radio_interfaces.h		\
		include/radiostation-config.h		\
		include/radiostation.h			\
		include/radiostation-listview.h		\
		include/ringbuffer.h			\
		include/seekhelper.h			\
		include/soundformat.h			\
		include/sound_metadata.h		\
		include/soundstreamclient_interfaces.h	\
		include/soundstreamid.h			\
		include/standardscandialog.h		\
		include/station-drag-object.h		\
		include/stationlist.h			\
		include/stationlistmetadata.h		\
		include/stationlistxmlhandler.h		\
		include/stationselection_interfaces.h	\
		include/stationselector.h		\
		include/timecontrol_interfaces.h	\
		include/utils.h				\
		include/widgetplugins.h

kradio_LDADD =  $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_KIO)

kradio_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIRC) -Wl,--export-dynamic

xdg_apps_DATA = kradio.desktop

messages: rc.cpp
	$(EXTRACTRC)       *.ui  >> rc.cpp
	$(XGETTEXT) rc.cpp *.cpp include/*.h -o po/kradio.pot
