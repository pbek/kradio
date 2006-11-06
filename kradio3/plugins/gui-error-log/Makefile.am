SUBDIRS = po .

INCLUDES = $(all_includes)
METASOURCES = AUTO

libkradio_LTLIBRARIES = liberror-log.la
liberror_log_la_SOURCES = errorlog.cpp
liberror_log_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)

noinst_HEADERS = errorlog.h

#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o po/kradio-gui-error-log.pot

messages: rc.cpp
	$(EXTRACTRC) *.rc *.ui >> rc.cpp
	$(XGETTEXT) rc.cpp *.h *.cpp -o po/kradio-gui-error-log.pot
