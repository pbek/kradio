SUBDIRS = po .

INCLUDES = $(all_includes)
METASOURCES = AUTO

libkradio_LTLIBRARIES = libstandard-display.la
libstandard_display_la_SOURCES = displaycfg.cpp displaycfg_interfaces.cpp \
                                 radioview-configuration.cpp radioview.cpp \
                                 radioview_element.cpp \
                                 radioview_frequencyradio.cpp \
                                 radioview_frequencyseeker.cpp \
                                 radioview_volume.cpp
libstandard_display_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)

noinst_HEADERS = displaycfg.h displaycfg_interfaces.h radioview-configuration.h radioview_element.h radioview_frequencyradio.h radioview_frequencyseeker.h radioview.h radioview_volume.h

#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o po/kradio-gui-standard-display.pot

messages: rc.cpp
	$(EXTRACTRC) *.rc *.ui >> rc.cpp
	$(XGETTEXT) rc.cpp *.h *.cpp -o po/kradio-gui-standard-display.pot
