SUBDIRS =
EXTRA_DIST = "amsterdam-cable.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/"
	$(INSTALL_DATA) "$(srcdir)/amsterdam-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/amsterdam-cable.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/amsterdam-cable.krp"
