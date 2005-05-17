SUBDIRS =
EXTRA_DIST = "brisbane-antenna.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/australia/"
	$(INSTALL_DATA) "$(srcdir)/brisbane-antenna.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/australia/brisbane-antenna.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/australia/brisbane-antenna.krp"
