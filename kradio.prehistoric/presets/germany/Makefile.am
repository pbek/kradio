SUBDIRS =
EXTRA_DIST = "aachen-cable.krp" "bonn-antenna.krp" "freiburg-cable.krp" "hamburg-antenna.krp" "muensterland-antenna.krp" "wernigerode-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(kde_datadir)/kradio/presets/germany/"
	$(INSTALL_DATA) "$(srcdir)/aachen-cable.krp" "$(kde_datadir)/kradio/presets/germany/aachen-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/bonn-antenna.krp" "$(kde_datadir)/kradio/presets/germany/bonn-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/hamburg-antenna.krp" "$(kde_datadir)/kradio/presets/germany/hamburg-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/wernigerode-antenna.krp" "$(kde_datadir)/kradio/presets/germany/wernigerode-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/freiburg-cable.krp" "$(kde_datadir)/kradio/presets/germany/freiburg-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/muensterland-antenna.krp" "$(kde_datadir)/kradio/presets/germany/muensterland-antenna.krp"


uninstall-local:
	-rm -f "$(kde_datadir)/kradio/presets/germany/aachen-cable.krp"
	-rm -f "$(kde_datadir)/kradio/presets/germany/bonn-antenna.krp"
	-rm -f "$(kde_datadir)/kradio/presets/germany/hamburg-antenna.krp"
	-rm -f "$(kde_datadir)/kradio/presets/germany/wernigerode-antenna.krp"
	-rm -f "$(kde_datadir)/kradio/presets/germany/freiburg-cable.krp"
	-rm -f "$(kde_datadir)/kradio/presets/germany/muensterland-antenna.krp"
