SUBDIRS =
EXTRA_DIST = "auckland-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/new-zealand/"
	$(INSTALL_DATA) "$(srcdir)/auckland-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/new-zealand/auckland-antenna.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/new-zealand/auckland-antenna.krp"
