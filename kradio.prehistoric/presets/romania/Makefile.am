SUBDIRS =
EXTRA_DIST = "brasov.krp" "bucharest.krp" "craiova.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/"
	$(INSTALL_DATA) "$(srcdir)/bucharest.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/bucharest.krp"
	$(INSTALL_DATA) "$(srcdir)/craiova.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/craiova.krp"
	$(INSTALL_DATA) "$(srcdir)/brasov.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/brasov.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/bucharest.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/craiova.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/brasov.krp"
