SUBDIRS = . libkradio interfaces libkradio-gui radio-stations
METASOURCES = AUTO
INCLUDES = $(all_includes)
KDE_CXXFLAGS="-UQT_NO_ASCII_CAST"
KDE_ICON = AUTO

bin_PROGRAMS = kradio

kradio_SOURCES = main.cpp
kradio_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIRC)
kradio_LDADD = $(top_builddir)/kradio/radio-stations/libradio-stations.la \
	$(top_builddir)/kradio/libkradio-gui/libkradio-gui.la $(top_builddir)/kradio/libkradio/libkradio.la \
	$(top_builddir)/kradio/interfaces/libinterfaces.la $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIBSOCKET)

xdg_apps_DATA = kradio.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kradio.pot
