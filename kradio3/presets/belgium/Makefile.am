SUBDIRS =
EXTRA_DIST = "limburg-cable.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/belgium/"
	$(INSTALL_DATA) "$(srcdir)/limburg-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/belgium/limburg-cable.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/belgium/limburg-cable.krp"
