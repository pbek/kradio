SUBDIRS = buckinghamshire
EXTRA_DIST = "leeds-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/england/"
	$(INSTALL_DATA) "$(srcdir)/leeds-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/england/leeds-antenna.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/england/leeds-antenna.krp"
