SUBDIRS =
EXTRA_DIST = "barcelona2.krp" "barcelona.krp" "sabadell-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/catalonia/"
	$(INSTALL_DATA) "$(srcdir)/barcelona.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/catalonia/barcelona.krp"
	$(INSTALL_DATA) "$(srcdir)/barcelona2.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/catalonia/barcelona2.krp"
	$(INSTALL_DATA) "$(srcdir)/sabadell-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/catalonia/sabadell-antenna.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/catalonia/barcelona.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/catalonia/barcelona2.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/catalonia/sabadell-antenna.krp"
