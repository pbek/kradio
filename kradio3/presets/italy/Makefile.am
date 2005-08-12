SUBDIRS =
EXTRA_DIST = "bologna.krp" "rovato.krp" "torino-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/italy/"
	$(INSTALL_DATA) "$(srcdir)/bologna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/italy/bologna.krp"
	$(INSTALL_DATA) "$(srcdir)/rovato.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/italy/rovato.krp"
	$(INSTALL_DATA) "$(srcdir)/torino-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/italy/torino-antenna.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/italy/bologna.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/italy/rovato.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/italy/torino-antenna.krp"
