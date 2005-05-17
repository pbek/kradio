SUBDIRS =
EXTRA_DIST = "paris-antenna.2.krp" "paris-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/france/"
	$(INSTALL_DATA) "$(srcdir)/paris-antenna.2.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/france/paris-antenna.2.krp"
	$(INSTALL_DATA) "$(srcdir)/paris-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/france/paris-antenna.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/france/paris-antenna.2.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/france/paris-antenna.krp"
