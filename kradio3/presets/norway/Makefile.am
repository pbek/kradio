SUBDIRS =
EXTRA_DIST = "oslo.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/norway/"
	$(INSTALL_DATA) "$(srcdir)/oslo.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/norway/oslo.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/norway/oslo.krp"
