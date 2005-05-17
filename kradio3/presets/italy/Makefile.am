SUBDIRS =
EXTRA_DIST = "bologna.krp" "torino-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/italy/"
	$(INSTALL_DATA) "$(srcdir)/torino-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/italy/torino-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/bologna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/italy/bologna.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/italy/torino-antenna.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/italy/bologna.krp"
