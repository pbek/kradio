SUBDIRS =
EXTRA_DIST = "bucharest.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/"
	$(INSTALL_DATA) "$(srcdir)/bucharest.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/bucharest.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/romania/bucharest.krp"
