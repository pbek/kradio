SUBDIRS = po icons .

INCLUDES = $(all_includes)
METASOURCES = AUTO

libkradio_LTLIBRARIES = libtimeshifter.la
libtimeshifter_la_SOURCES = timeshifter.cpp timeshifter-configuration-ui.ui \
                            timeshifter-configuration.cpp
libtimeshifter_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)

noinst_HEADERS = timeshifter.h timeshifter-configuration.h

#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o po/kradio-timeshifter.pot

messages: rc.cpp
	$(EXTRACTRC) *.rc *.ui >> rc.cpp
	$(XGETTEXT) rc.cpp *.h *.cpp -o po/kradio-timeshifter.pot
