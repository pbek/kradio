SUBDIRS =
EXTRA_DIST = "angers.krp" "lille-antenna.krp" "paris-antenna.2.krp" "paris-antenna.krp" "rennes.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/france/"
	$(INSTALL_DATA) "$(srcdir)/paris-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/france/paris-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/angers.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/france/angers.krp"
	$(INSTALL_DATA) "$(srcdir)/lille-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/france/lille-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/rennes.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/france/rennes.krp"
	$(INSTALL_DATA) "$(srcdir)/paris-antenna.2.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/france/paris-antenna.2.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/france/paris-antenna.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/france/angers.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/france/lille-antenna.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/france/rennes.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/france/paris-antenna.2.krp"
