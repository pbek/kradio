SUBDIRS =
EXTRA_DIST = "elblang-antenna-fm.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/"
	$(INSTALL_DATA) "$(srcdir)/elblang-antenna-fm.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/elblang-antenna-fm.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/poland/elblang-antenna-fm.krp"
