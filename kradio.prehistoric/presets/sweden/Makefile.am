SUBDIRS =
EXTRA_DIST = "gothenburg.krp" "oestersunds-kommun.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/"
	$(INSTALL_DATA) "$(srcdir)/oestersunds-kommun.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/oestersunds-kommun.krp"
	$(INSTALL_DATA) "$(srcdir)/gothenburg.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/gothenburg.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/oestersunds-kommun.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/gothenburg.krp"
