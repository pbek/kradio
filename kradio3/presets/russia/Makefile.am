SUBDIRS =
EXTRA_DIST = "moscow.krp" "saint-petersburg-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/russia/"
	$(INSTALL_DATA) "$(srcdir)/saint-petersburg-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/russia/saint-petersburg-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/moscow.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/russia/moscow.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/russia/saint-petersburg-antenna.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/russia/moscow.krp"
