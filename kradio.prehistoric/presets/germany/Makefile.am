SUBDIRS =
EXTRA_DIST = "aachen-cable.krp" "bonn-antenna.krp" "freiburg-cable.krp" "hamburg-antenna.krp" "muensterland-antenna.krp" "wernigerode-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/germany/"
	$(INSTALL_DATA) "$(srcdir)/aachen-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/germany/aachen-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/bonn-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/germany/bonn-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/hamburg-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/germany/hamburg-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/wernigerode-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/germany/wernigerode-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/freiburg-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/germany/freiburg-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/muensterland-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/germany/muensterland-antenna.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/germany/aachen-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/germany/bonn-antenna.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/germany/hamburg-antenna.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/germany/wernigerode-antenna.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/germany/freiburg-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/germany/muensterland-antenna.krp"
