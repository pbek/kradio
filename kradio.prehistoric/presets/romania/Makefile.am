SUBDIRS =
EXTRA_DIST = "bucharest.krp" "craiova.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/"
	$(INSTALL_DATA) "$(srcdir)/bucharest.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/bucharest.krp"
	$(INSTALL_DATA) "$(srcdir)/craiova.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/craiova.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/bucharest.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/craiova.krp"
