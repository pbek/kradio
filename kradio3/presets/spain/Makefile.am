SUBDIRS =
EXTRA_DIST = "barcelona2.krp" "barcelona.krp" "bilbao.krp" "lugo.krp" "madrid-2.krp" "madrid.krp" "malaga.krp" "mieres-del-camino.krp" "sevilla.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/"
	$(INSTALL_DATA) "$(srcdir)/sevilla.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/sevilla.krp"
	$(INSTALL_DATA) "$(srcdir)/malaga.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/malaga.krp"
	$(INSTALL_DATA) "$(srcdir)/bilbao.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/bilbao.krp"
	$(INSTALL_DATA) "$(srcdir)/barcelona2.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/barcelona2.krp"
	$(INSTALL_DATA) "$(srcdir)/lugo.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/lugo.krp"
	$(INSTALL_DATA) "$(srcdir)/madrid-2.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/madrid-2.krp"
	$(INSTALL_DATA) "$(srcdir)/mieres-del-camino.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/mieres-del-camino.krp"
	$(INSTALL_DATA) "$(srcdir)/barcelona.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/barcelona.krp"
	$(INSTALL_DATA) "$(srcdir)/madrid.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/madrid.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/sevilla.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/malaga.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/bilbao.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/barcelona2.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/lugo.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/madrid-2.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/mieres-del-camino.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/barcelona.krp"
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/spain/madrid.krp"
