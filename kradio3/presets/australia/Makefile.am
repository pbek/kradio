SUBDIRS =
EXTRA_DIST = "brisbane-antenna-2.krp" "brisbane-antenna.krp" "brisbane.krp" "sydney.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/australia/"
	$(INSTALL_DATA) "$(srcdir)/sydney.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/australia/sydney.krp"
	$(INSTALL_DATA) "$(srcdir)/brisbane-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/australia/brisbane-antenna.krp"
	$(INSTALL_DATA) "$(srcdir)/brisbane-antenna-2.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/australia/brisbane-antenna-2.krp"
	$(INSTALL_DATA) "$(srcdir)/brisbane.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/australia/brisbane.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/australia/sydney.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/australia/brisbane-antenna.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/australia/brisbane-antenna-2.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/australia/brisbane.krp"
