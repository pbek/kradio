SUBDIRS =
EXTRA_DIST = "diepoldsau-cable.krp" "stansstad-cable.krp" "wrenlos-cable.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/switzerland/"
	$(INSTALL_DATA) "$(srcdir)/diepoldsau-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/switzerland/diepoldsau-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/wrenlos-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/switzerland/wrenlos-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/stansstad-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/switzerland/stansstad-cable.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/switzerland/diepoldsau-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/switzerland/wrenlos-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/switzerland/stansstad-cable.krp"
