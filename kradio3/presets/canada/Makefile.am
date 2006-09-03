SUBDIRS =
EXTRA_DIST = "montreal-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/canada/"
	$(INSTALL_DATA) "$(srcdir)/montreal-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/canada/montreal-antenna.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/canada/montreal-antenna.krp"
