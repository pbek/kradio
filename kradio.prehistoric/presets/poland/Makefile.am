SUBDIRS =
EXTRA_DIST = "biaystock-antenna.krp" "elblang-antenna-fm.krp" "krakow-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/"
	$(INSTALL_DATA) "$(srcdir)/elblang-antenna-fm.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/elblang-antenna-fm.krp"
	$(INSTALL_DATA) "$(srcdir)/krakow-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/krakow-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/biaystock-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/biaystock-antenna.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/elblang-antenna-fm.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/krakow-antenna.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/biaystock-antenna.krp"
