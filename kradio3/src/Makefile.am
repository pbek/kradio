SUBDIRS = . libkradio interfaces libkradio-gui radio-stations
METASOURCES = AUTO
INCLUDES = $(all_includes)
KDE_CXXFLAGS="-UQT_NO_ASCII_CAST"
KDE_ICON = AUTO

bin_PROGRAMS = kradio

kradio_SOURCES = main.cpp
kradio_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIRC)
kradio_LDADD = radio-stations/libkradio-stations.la \
	libkradio-gui/libkradio-gui.la libkradio/libkradio.la \
	interfaces/libkradio-interfaces.la \
	$(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIBSOCKET)

xdg_apps_DATA = kradio.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kradio.pot
