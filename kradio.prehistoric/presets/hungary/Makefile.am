SUBDIRS =
EXTRA_DIST = "budapest2.krp" "budapest.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/hungary/"
	$(INSTALL_DATA) "$(srcdir)/budapest.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/hungary/budapest.krp"
	$(INSTALL_DATA) "$(srcdir)/budapest2.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/hungary/budapest2.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/hungary/budapest.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/hungary/budapest2.krp"
