####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = aachen-cable.krp hamburg-antenna.krp 

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kradio/presets/germany/
	$(INSTALL_DATA) $(srcdir)/aachen-cable.krp $(kde_datadir)/kradio/presets/germany/aachen-cable.krp
	$(mkinstalldirs) $(kde_datadir)/kradio/presets/germany/
	$(INSTALL_DATA) $(srcdir)/hamburg-antenna.krp $(kde_datadir)/kradio/presets/germany/hamburg-antenna.krp

uninstall-local:
	-rm -f $(kde_datadir)/kradio/presets/germany/aachen-cable.krp
	-rm -f $(kde_datadir)/kradio/presets/germany/hamburg-antenna.krp

####### kdevelop will overwrite this part!!! (end)############
