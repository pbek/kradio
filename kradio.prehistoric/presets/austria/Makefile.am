SUBDIRS =
EXTRA_DIST = "vienna-antenna.krp" "vienna-cable.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/"
	$(INSTALL_DATA) "$(srcdir)/vienna-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/vienna-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/vienna-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/vienna-antenna.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/vienna-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/vienna-antenna.krp"
