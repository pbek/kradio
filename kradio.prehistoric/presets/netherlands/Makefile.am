SUBDIRS =
EXTRA_DIST = "alphen-aan-den-rijn-cable.krp" "amersfoort-cable.krp" "amsterdam-cable.krp" "arnhem-cable.krp" "rotterdam-cable.krp" "tegelen-cable.krp" "the-hague-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/"
	$(INSTALL_DATA) "$(srcdir)/amsterdam-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/amsterdam-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/tegelen-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/tegelen-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/alphen-aan-den-rijn-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/alphen-aan-den-rijn-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/rotterdam-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/rotterdam-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/amersfoort-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/amersfoort-cable.krp"
	$(INSTALL_DATA) "$(srcdir)/the-hague-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/the-hague-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/arnhem-cable.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/arnhem-cable.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/amsterdam-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/tegelen-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/alphen-aan-den-rijn-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/rotterdam-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/amersfoort-cable.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/the-hague-antenna.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/netherlands/arnhem-cable.krp"
