SUBDIRS =
EXTRA_DIST = "plzen-cable.krp" "prague-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/czechia/"
	$(INSTALL_DATA) "$(srcdir)/plzen-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/czechia/plzen-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/prague-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/czechia/prague-antenna.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/czechia/plzen-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/czechia/prague-antenna.krp"
