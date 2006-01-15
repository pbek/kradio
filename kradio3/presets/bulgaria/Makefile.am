SUBDIRS =
EXTRA_DIST = "sofia.krp" "stara-zagora.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/bulgaria/"
	$(INSTALL_DATA) "$(srcdir)/sofia.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/bulgaria/sofia.krp"
	$(INSTALL_DATA) "$(srcdir)/stara-zagora.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/bulgaria/stara-zagora.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/bulgaria/sofia.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/bulgaria/stara-zagora.krp"
