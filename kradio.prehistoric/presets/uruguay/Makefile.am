SUBDIRS =
EXTRA_DIST = "montevideo.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/uruguay/"
	$(INSTALL_DATA) "$(srcdir)/montevideo.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/uruguay/montevideo.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/uruguay/montevideo.krp"
