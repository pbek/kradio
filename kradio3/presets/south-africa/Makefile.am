SUBDIRS =
EXTRA_DIST = "johannesburg.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/south-africa/"
	$(INSTALL_DATA) "$(srcdir)/johannesburg.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/south-africa/johannesburg.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/south-africa/johannesburg.krp"
