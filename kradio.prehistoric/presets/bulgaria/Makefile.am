SUBDIRS =
EXTRA_DIST = "sofia.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/bulgaria/"
	$(INSTALL_DATA) "$(srcdir)/sofia.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/bulgaria/sofia.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/bulgaria/sofia.krp"
