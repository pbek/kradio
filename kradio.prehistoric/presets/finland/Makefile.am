SUBDIRS =
EXTRA_DIST = "helsinki-cable.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/finland/"
	$(INSTALL_DATA) "$(srcdir)/helsinki-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/finland/helsinki-cable.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/finland/helsinki-cable.krp"
