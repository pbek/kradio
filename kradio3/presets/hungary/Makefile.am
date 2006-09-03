SUBDIRS =
EXTRA_DIST = "budapest-antenna.2.krp" "budapest-antenna.krp" "budapest.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/hungary/"
	$(INSTALL_DATA) "$(srcdir)/budapest.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/hungary/budapest.krp"
	$(INSTALL_DATA) "$(srcdir)/budapest-antenna.2.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/hungary/budapest-antenna.2.krp"
	$(INSTALL_DATA) "$(srcdir)/budapest-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/hungary/budapest-antenna.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/hungary/budapest.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/hungary/budapest-antenna.2.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/hungary/budapest-antenna.krp"
