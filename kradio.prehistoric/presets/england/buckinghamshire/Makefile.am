EXTRA_DIST = milton_keynes.krp

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kradio/presets/england/buckinghamshire/
	$(INSTALL_DATA) $(srcdir)/milton_keynes.krp $(kde_datadir)/kradio/presets/england/buckinghamshire/milton_keynes.krp


uninstall-local:
	-rm -f $(kde_datadir)/kradio/presets/england/buckinghamshire/milton_keynes.krp
