SUBDIRS =
EXTRA_DIST = "gothenburg.krp" "oestersunds-kommun.krp" "stockholm.2.krp" "stockholm.krp" "vaxholm.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/"
	$(INSTALL_DATA) "$(srcdir)/gothenburg.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/gothenburg.krp"
	$(INSTALL_DATA) "$(srcdir)/oestersunds-kommun.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/oestersunds-kommun.krp"
	$(INSTALL_DATA) "$(srcdir)/stockholm.2.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/stockholm.2.krp"
	$(INSTALL_DATA) "$(srcdir)/stockholm.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/stockholm.krp"
	$(INSTALL_DATA) "$(srcdir)/vaxholm.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/vaxholm.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/gothenburg.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/oestersunds-kommun.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/stockholm.2.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/stockholm.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/vaxholm.krp"
