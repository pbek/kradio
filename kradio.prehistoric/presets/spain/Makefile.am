SUBDIRS =
EXTRA_DIST = "barcelona.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/"
	$(INSTALL_DATA) "$(srcdir)/barcelona.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/barcelona.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/barcelona.krp"
