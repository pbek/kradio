SUBDIRS =
EXTRA_DIST = "gothenburg.krp" "oestersunds-kommun.krp" "stockholm.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/"
	$(INSTALL_DATA) "$(srcdir)/oestersunds-kommun.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/oestersunds-kommun.krp"
	$(INSTALL_DATA) "$(srcdir)/gothenburg.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/gothenburg.krp"
	$(INSTALL_DATA) "$(srcdir)/stockholm.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/stockholm.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/oestersunds-kommun.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/gothenburg.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/stockholm.krp"
