SUBDIRS =
EXTRA_DIST = "breitenfurt-cable.krp" "gablitz-cable.krp" "kaltenleutgeben-cable.krp" "laab-wald-cable.krp" "mauerbach-cable.krp" "purkersdorf-cable.krp" "vienna-antenna.krp" "vienna-cable.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/"
	$(INSTALL_DATA) "$(srcdir)/vienna-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/vienna-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/vienna-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/vienna-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/purkersdorf-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/purkersdorf-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/mauerbach-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/mauerbach-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/gablitz-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/gablitz-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/laab-wald-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/laab-wald-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/breitenfurt-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/breitenfurt-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/kaltenleutgeben-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/kaltenleutgeben-cable.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/vienna-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/vienna-antenna.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/purkersdorf-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/mauerbach-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/gablitz-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/laab-wald-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/breitenfurt-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/austria/kaltenleutgeben-cable.krp"
