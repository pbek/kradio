SUBDIRS = buckinghamshire
EXTRA_DIST = "exeter-antenna.krp" "leeds-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/england/"
	$(INSTALL_DATA) "$(srcdir)/leeds-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/england/leeds-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/exeter-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/england/exeter-antenna.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/england/leeds-antenna.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/england/exeter-antenna.krp"
