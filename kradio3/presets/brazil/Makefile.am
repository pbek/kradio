SUBDIRS =
EXTRA_DIST = "sao-paulo.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/brazil/"
	$(INSTALL_DATA) "$(srcdir)/sao-paulo.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/brazil/sao-paulo.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/brazil/sao-paulo.krp"
