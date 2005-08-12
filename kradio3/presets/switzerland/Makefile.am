SUBDIRS =
EXTRA_DIST = "stansstad-cable.krp" "wrenlos-cable.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/switzerland/"
	$(INSTALL_DATA) "$(srcdir)/stansstad-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/switzerland/stansstad-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/wrenlos-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/switzerland/wrenlos-cable.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/switzerland/stansstad-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/switzerland/wrenlos-cable.krp"
