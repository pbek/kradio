SUBDIRS =
EXTRA_DIST = "dublin.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/ireland/"
	$(INSTALL_DATA) "$(srcdir)/dublin.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/ireland/dublin.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/ireland/dublin.krp"
