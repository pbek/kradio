SUBDIRS =
EXTRA_DIST = "plzen-cable.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/czechia/"
	$(INSTALL_DATA) "$(srcdir)/plzen-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/czechia/plzen-cable.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/czechia/plzen-cable.krp"
