SUBDIRS =
EXTRA_DIST = "torino-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/italy/"
	$(INSTALL_DATA) "$(srcdir)/torino-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/italy/torino-antenna.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/italy/torino-antenna.krp"
