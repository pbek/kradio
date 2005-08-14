INCLUDES = $(all_includes)
METASOURCES = AUTO

libkradio_LTLIBRARIES = liblirc.la
liblirc_la_SOURCES = lircsupport.cpp lirc-configuration-ui.ui \
	lirc-configuration.cpp listviewitem_lirc.cpp
liblirc_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)
liblirc_la_LIBADD = $(LIB_LIRC)

noinst_HEADERS = lircsupport.h lirc-configuration.h listviewitem_lirc.h


install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/"
	$(INSTALL_DATA) "$(srcdir)/default-dot-lircrc" "$(DESTDIR)$(kde_datadir)/kradio/default-dot-lircrc"

uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/default-dot-lircrc"
