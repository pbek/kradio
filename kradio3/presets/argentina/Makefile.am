SUBDIRS =
EXTRA_DIST = "buenos-aires.krp"

install-data-local:
	$(mkinstalldirs) "$(DESTDIR)$(kde_datadir)/kradio/presets/argentina/"
	$(INSTALL_DATA) "$(srcdir)/buenos-aires.krp" "$(DESTDIR)$(kde_datadir)/kradio/presets/argentina/buenos-aires.krp"


uninstall-local:
	-rm -f "$(DESTDIR)$(kde_datadir)/kradio/presets/argentina/buenos-aires.krp"
