SUBDIRS =
EXTRA_DIST = "budapest.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/hungary/"
	$(INSTALL_DATA) "$(srcdir)/budapest.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/hungary/budapest.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/hungary/budapest.krp"
