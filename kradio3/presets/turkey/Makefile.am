SUBDIRS =
EXTRA_DIST = "adapazari-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/turkey/"
	$(INSTALL_DATA) "$(srcdir)/adapazari-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/turkey/adapazari-antenna.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/turkey/adapazari-antenna.krp"
