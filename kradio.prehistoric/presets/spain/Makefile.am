SUBDIRS =
EXTRA_DIST = "barcelona.krp" "madrid.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/"
	$(INSTALL_DATA) "$(srcdir)/barcelona.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/barcelona.krp"
	$(INSTALL_DATA) "$(srcdir)/madrid.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/madrid.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/barcelona.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/madrid.krp"
