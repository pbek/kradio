SUBDIRS =
EXTRA_DIST = "alphen-aan-den-rijn-cable.krp" "amersfoort-cable.krp" "amsterdam-cable.krp" "tegelen-cable.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/"
	$(INSTALL_DATA) "$(srcdir)/amsterdam-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/amsterdam-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/tegelen-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/tegelen-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/alphen-aan-den-rijn-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/alphen-aan-den-rijn-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/amersfoort-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/amersfoort-cable.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/amsterdam-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/tegelen-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/alphen-aan-den-rijn-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/amersfoort-cable.krp"
