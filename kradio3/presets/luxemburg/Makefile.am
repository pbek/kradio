SUBDIRS =
EXTRA_DIST = "walferdange.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/luxemburg/"
	$(INSTALL_DATA) "$(srcdir)/walferdange.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/luxemburg/walferdange.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/luxemburg/walferdange.krp"
