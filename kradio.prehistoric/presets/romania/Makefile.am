SUBDIRS =
EXTRA_DIST = "bucharest.krp"

install-data-local:
	$(mkinstalldirs) "$(kde_datadir)/kradio/presets/romania/"
	$(INSTALL_DATA) "$(srcdir)/bucharest.krp" "$(kde_datadir)/kradio/presets/romania/bucharest.krp"


uninstall-local:
	-rm -f "$(kde_datadir)/kradio/presets/romania/bucharest.krp"
