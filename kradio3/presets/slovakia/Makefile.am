SUBDIRS =
EXTRA_DIST = "kosice.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/slovakia/"
	$(INSTALL_DATA) "$(srcdir)/kosice.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/slovakia/kosice.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/slovakia/kosice.krp"
