SUBDIRS =
EXTRA_DIST = "barcelona2.krp" "barcelona.krp" "bilbao.krp" "madrid.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/"
	$(INSTALL_DATA) "$(srcdir)/barcelona.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/barcelona.krp"
	$(INSTALL_DATA) "$(srcdir)/madrid.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/madrid.krp"
	$(INSTALL_DATA) "$(srcdir)/barcelona2.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/barcelona2.krp"
	$(INSTALL_DATA) "$(srcdir)/bilbao.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/bilbao.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/barcelona.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/madrid.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/barcelona2.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/bilbao.krp"
