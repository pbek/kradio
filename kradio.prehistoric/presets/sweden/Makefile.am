SUBDIRS =
EXTRA_DIST = "oestersunds-kommun.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/"
	$(INSTALL_DATA) "$(srcdir)/oestersunds-kommun.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/oestersunds-kommun.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/sweden/oestersunds-kommun.krp"
