SUBDIRS =
EXTRA_DIST = "buellingen-antenna.krp" "ieper-cable.krp" "limburg-cable.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/belgium/"
	$(INSTALL_DATA) "$(srcdir)/limburg-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/belgium/limburg-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/buellingen-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/belgium/buellingen-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/ieper-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/belgium/ieper-cable.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/belgium/limburg-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/belgium/buellingen-antenna.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/belgium/ieper-cable.krp"
