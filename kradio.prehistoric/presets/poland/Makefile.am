SUBDIRS =
EXTRA_DIST = "bialystock-antenna.krp" "elblang-antenna-fm.krp" "krakow-antenna.krp" "warsaw-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/"
	$(INSTALL_DATA) "$(srcdir)/warsaw-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/warsaw-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/elblang-antenna-fm.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/elblang-antenna-fm.krp"
	$(INSTALL_DATA) "$(srcdir)/krakow-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/krakow-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/bialystock-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/bialystock-antenna.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/warsaw-antenna.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/elblang-antenna-fm.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/krakow-antenna.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/bialystock-antenna.krp"
