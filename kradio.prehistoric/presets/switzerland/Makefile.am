SUBDIRS =
EXTRA_DIST = "wrenlos-cable.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/switzerland/"
	$(INSTALL_DATA) "$(srcdir)/wrenlos-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/switzerland/wrenlos-cable.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/switzerland/wrenlos-cable.krp"
