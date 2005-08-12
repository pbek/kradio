SUBDIRS =
EXTRA_DIST = "rio-de-janeiro.krp" "sao-paulo.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/brazil/"
	$(INSTALL_DATA) "$(srcdir)/rio-de-janeiro.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/brazil/rio-de-janeiro.krp"
	$(INSTALL_DATA) "$(srcdir)/sao-paulo.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/brazil/sao-paulo.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/brazil/rio-de-janeiro.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/brazil/sao-paulo.krp"
