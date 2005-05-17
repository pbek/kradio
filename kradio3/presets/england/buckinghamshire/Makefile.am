SUBDIRS =
EXTRA_DIST = "milton_keynes.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/england/buckinghamshire/"
	$(INSTALL_DATA) "$(srcdir)/milton_keynes.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/england/buckinghamshire/milton_keynes.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/england/buckinghamshire/milton_keynes.krp"
