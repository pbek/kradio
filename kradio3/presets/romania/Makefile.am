SUBDIRS =
EXTRA_DIST = "brasov-2.krp" "brasov.krp" "bucharest.krp" "bucuresti.krp" "craiova.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/"
	$(INSTALL_DATA) "$(srcdir)/bucharest.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/bucharest.krp"
	$(INSTALL_DATA) "$(srcdir)/brasov.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/brasov.krp"
	$(INSTALL_DATA) "$(srcdir)/brasov-2.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/brasov-2.krp"
	$(INSTALL_DATA) "$(srcdir)/craiova.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/craiova.krp"
	$(INSTALL_DATA) "$(srcdir)/bucuresti.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/bucuresti.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/bucharest.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/brasov.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/brasov-2.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/craiova.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/bucuresti.krp"
