SUBDIRS =
EXTRA_DIST = "wrenlos-cable.krp"

install-data-local:
	$(mkinstalldirs) "$(kde_datadir)/kradio/presets/switzerland/"
	$(INSTALL_DATA) "$(srcdir)/wrenlos-cable.krp" "$(kde_datadir)/kradio/presets/switzerland/wrenlos-cable.krp"


uninstall-local:
	-rm -f "$(kde_datadir)/kradio/presets/switzerland/wrenlos-cable.krp"
