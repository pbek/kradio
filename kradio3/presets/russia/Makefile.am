SUBDIRS =
EXTRA_DIST = "moscow.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/russia/"
	$(INSTALL_DATA) "$(srcdir)/moscow.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/russia/moscow.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/russia/moscow.krp"
