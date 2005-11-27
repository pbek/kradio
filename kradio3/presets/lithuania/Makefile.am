SUBDIRS =
EXTRA_DIST = "kaunas-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/lithuania/"
	$(INSTALL_DATA) "$(srcdir)/kaunas-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/lithuania/kaunas-antenna.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/lithuania/kaunas-antenna.krp"
