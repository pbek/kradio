INCLUDES = -I$(top_srcdir)/kradio3/src -I$(top_builddir)/kradio3/src $(all_includes)
METASOURCES = AUTO
noinst_LTLIBRARIES =  libkradio-gui.la
libkradio_gui_la_LDFLAGS = $(all_libraries) --export-dynamic --export-all-symbols
noinst_HEADERS = radiostation-listview.h standardscandialog.h stationselector.h \
	gui_list_helper.h
libkradio_gui_la_SOURCES = radiostation-listview.cpp standardscandialog.cpp \
		stationselector.cpp standardscandialog-ui.ui stationselector-ui.ui aboutwidget.cpp

