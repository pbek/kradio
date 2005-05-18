SUBDIRS =
EXTRA_DIST = "bialystock-antenna.krp" "elblang-antenna-fm.krp" "katowice.krp" "krakow-antenna.krp" "torun-cable.krp" "warsaw-antenna.2.krp" "warsaw-antenna.krp" "warsaw.krp" "warsaw-upc_cable.krp" "warszawa-cable.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/"
	$(INSTALL_DATA) "$(srcdir)/krakow-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/krakow-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/torun-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/torun-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/bialystock-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/bialystock-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/elblang-antenna-fm.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/elblang-antenna-fm.krp"
	$(INSTALL_DATA) "$(srcdir)/warsaw-antenna.2.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/warsaw-antenna.2.krp"
	$(INSTALL_DATA) "$(srcdir)/warsaw-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/warsaw-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/warsaw-upc_cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/warsaw-upc_cable.krp"
	$(INSTALL_DATA) "$(srcdir)/warszawa-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/warszawa-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/warsaw.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/warsaw.krp"
	$(INSTALL_DATA) "$(srcdir)/katowice.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/katowice.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/krakow-antenna.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/torun-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/bialystock-antenna.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/elblang-antenna-fm.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/warsaw-antenna.2.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/warsaw-antenna.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/warsaw-upc_cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/warszawa-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/warsaw.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/katowice.krp"
