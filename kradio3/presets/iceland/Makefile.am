SUBDIRS =
EXTRA_DIST = "reykjavik-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/iceland/"
	$(INSTALL_DATA) "$(srcdir)/reykjavik-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/iceland/reykjavik-antenna.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/iceland/reykjavik-antenna.krp"
