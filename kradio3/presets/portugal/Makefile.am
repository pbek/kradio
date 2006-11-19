SUBDIRS =
EXTRA_DIST = "lisboa-antenna.krp aveiro-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/portugal/"
	$(INSTALL_DATA) "$(srcdir)/lisboa-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/portugal/lisboa-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/aveiro-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/portugal/aveiro-antenna.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/portugal/lisboa-antenna.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/portugal/aveiro-antenna.krp"
