####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = kradio

## INCLUDES were found outside kdevelop specific part

kradio_SOURCES = errorlog-interfaces.cpp errorlog.cpp aboutwidget.cpp recording-datamonitor.cpp recording-monitor.cpp recording-configuration.cpp recording-configuration-ui.ui recording-context.cpp recording-interfaces.cpp recording.cpp widgetplugins.cpp radiostation-config.cpp displaycfg.cpp displaycfg_interfaces.cpp radioview-configuration.cpp standardscandialog.cpp standardscandialog-ui.ui radio-configuration.cpp radio-configuration-ui.ui quickbar-configuration.cpp stationselector.cpp stationselector-ui.ui stationselection_interfaces.cpp docking-configuration.cpp docking-configuration-ui.ui timecontrol-configuration.cpp timecontrol-configuration-ui.ui plugin_configuration_dialog.cpp v4lradio-configuration.cpp v4lradio-configuration-ui.ui v4lcfg_interfaces.cpp radioview_element.cpp radioview_frequencyseeker.cpp radioview_volume.cpp radioview_frequencyradio.cpp radioview.cpp docking.cpp quickbar.cpp buttonflowlayout.cpp radio.cpp seekhelper.cpp frequencyseekhelper.cpp pluginmanager.cpp stationlistxmlhandler.cpp radiodevice_interfaces.cpp radiodevicepool_interface.cpp stationlist.cpp internetradiostation.cpp frequencyradiostation.cpp radio_interfaces.cpp timecontrol_interfaces.cpp plugins.cpp timecontrol.cpp utils.cpp lircsupport.cpp alarm.cpp radiostation.cpp v4lradio.cpp kradioapp.cpp main.cpp radiostation-listview.cpp

kradio_LDADD   =  $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIBSOCKET)

EXTRA_DIST = kradio.desktop lo32-app-kradio.png lo16-app-kradio.png lo22-app-kradio.png hi16-app-kradio.png hi22-app-kradio.png hi32-app-kradio.png dot-lircrc.example kradiorc.example hi24-app-kradio.png lo24-app-kradio.png hi48-app-kradio.png hi64-app-kradio.png lo48-app-kradio.png lo64-app-kradio.png kradio_muteoff.png kradio_muteon.png kradio_muteoff_16x16.png kradio_muteon_16x16.png v4lradio-configuration-ui.ui timecontrol-configuration-ui.ui docking-configuration-ui.ui stationselector-ui.ui radio-configuration-ui.ui standardscandialog-ui.ui recording-configuration-ui.ui kradio_record-16.png kradio_record-22.png kradio_record-24.png kradio_record-32.png kradio_record-256.png kradio_record-48.png kradio_record-64.png kradio_kalarm_16.png kradio_kalarm_22.png kradio_kalarm_32.png kradio_kalarm_48.png kradio-zzz-16x16.png kradio-zzz-22x22.png kradio-zzz-32x32.png kradio-zzz-48x48.png

noinst_HEADERS = radiostation-listview.h kradioapp.h v4lradio.h dot-lircrc.example kradiorc.example radiostation.h utils.h alarm.h lircsupport.h timecontrol.h interfaces.h radio_interfaces.h plugins.h timecontrol_interfaces.h frequencyradiostation.h internetradiostation.h radiodevice_interfaces.h stationlistmetadata.h radiodevicepool_interface.h stationlist.h stationlistxmlhandler.h pluginmanager.h frequencyseekhelper.h seekhelper.h radio.h buttonflowlayout.h quickbar.h docking.h radioview.h radioview_frequencyradio.h radioview_volume.h radioview_frequencyseeker.h radioview_element.h v4lradio-configuration.h v4lcfg_interfaces.h v4lradio-configuration-ui.ui plugin_configuration_dialog.h timecontrol-configuration-ui.ui timecontrol-configuration.h docking-configuration-ui.ui docking-configuration.h stationselection_interfaces.h stationselector-ui.ui stationselector.h quickbar-configuration.h radio-configuration-ui.ui radio-configuration.h standardscandialog-ui.ui standardscandialog.h radioview-configuration.h displaycfg_interfaces.h displaycfg.h radiostation-config.h widgetplugins.h recording.h recording-interfaces.h recording-context.h recording-configuration-ui.ui recording-configuration.h recording-monitor.h recording-datamonitor.h aboutwidget.h errorlog.h errorlog-interfaces.h

install-data-local:
	$(mkinstalldirs) $(kde_appsdir)/Applications/
	$(mkinstalldirs) $(kde_icondir)/locolor/16x16/apps/
	$(mkinstalldirs) $(kde_icondir)/locolor/22x22/apps/
	$(mkinstalldirs) $(kde_icondir)/locolor/24x24/apps/
	$(mkinstalldirs) $(kde_icondir)/locolor/32x32/apps/
	$(mkinstalldirs) $(kde_icondir)/locolor/48x48/apps/
	$(mkinstalldirs) $(kde_icondir)/locolor/64x64/apps/
	$(mkinstalldirs) $(kde_icondir)/hicolor/16x16/apps/
	$(mkinstalldirs) $(kde_icondir)/hicolor/22x22/apps/
	$(mkinstalldirs) $(kde_icondir)/hicolor/24x24/apps/
	$(mkinstalldirs) $(kde_icondir)/hicolor/32x32/apps/
	$(mkinstalldirs) $(kde_icondir)/hicolor/48x48/apps/
	$(mkinstalldirs) $(kde_icondir)/hicolor/64x64/apps/
	$(INSTALL_DATA) $(srcdir)/kradio.desktop       $(kde_appsdir)/Applications/kradio.desktop
	$(INSTALL_DATA) $(srcdir)/lo32-app-kradio.png  $(kde_icondir)/locolor/32x32/apps/kradio.png
	$(INSTALL_DATA) $(srcdir)/lo16-app-kradio.png  $(kde_icondir)/locolor/16x16/apps/kradio.png
	$(INSTALL_DATA) $(srcdir)/lo22-app-kradio.png  $(kde_icondir)/locolor/22x22/apps/kradio.png
	$(INSTALL_DATA) $(srcdir)/lo24-app-kradio.png  $(kde_icondir)/locolor/24x24/apps/kradio.png
	$(INSTALL_DATA) $(srcdir)/lo48-app-kradio.png  $(kde_icondir)/locolor/48x48/apps/kradio.png
	$(INSTALL_DATA) $(srcdir)/lo64-app-kradio.png  $(kde_icondir)/locolor/64x64/apps/kradio.png
	$(INSTALL_DATA) $(srcdir)/hi16-app-kradio.png  $(kde_icondir)/hicolor/16x16/apps/kradio.png
	$(INSTALL_DATA) $(srcdir)/hi22-app-kradio.png  $(kde_icondir)/hicolor/22x22/apps/kradio.png
	$(INSTALL_DATA) $(srcdir)/hi24-app-kradio.png  $(kde_icondir)/hicolor/24x24/apps/kradio.png
	$(INSTALL_DATA) $(srcdir)/hi32-app-kradio.png  $(kde_icondir)/hicolor/32x32/apps/kradio.png
	$(INSTALL_DATA) $(srcdir)/hi48-app-kradio.png  $(kde_icondir)/hicolor/48x48/apps/kradio.png
	$(INSTALL_DATA) $(srcdir)/hi64-app-kradio.png  $(kde_icondir)/hicolor/64x64/apps/kradio.png
	$(INSTALL_DATA) $(srcdir)/kradio_muteoff.png       $(kde_icondir)/hicolor/22x22/apps/kradio_muteoff.png
	$(INSTALL_DATA) $(srcdir)/kradio_muteon.png        $(kde_icondir)/hicolor/22x22/apps/kradio_muteon.png
	$(INSTALL_DATA) $(srcdir)/kradio_muteoff_16x16.png $(kde_icondir)/hicolor/16x16/apps/kradio_muteoff.png
	$(INSTALL_DATA) $(srcdir)/kradio_muteon_16x16.png  $(kde_icondir)/hicolor/16x16/apps/kradio_muteon.png
	$(INSTALL_DATA) $(srcdir)/kradio_record-16.png $(kde_icondir)/hicolor/16x16/apps/kradio_record.png
	$(INSTALL_DATA) $(srcdir)/kradio_record-22.png $(kde_icondir)/hicolor/22x22/apps/kradio_record.png
	$(INSTALL_DATA) $(srcdir)/kradio_record-24.png $(kde_icondir)/hicolor/24x24/apps/kradio_record.png
	$(INSTALL_DATA) $(srcdir)/kradio_record-32.png $(kde_icondir)/hicolor/32x32/apps/kradio_record.png
	$(INSTALL_DATA) $(srcdir)/kradio_record-48.png $(kde_icondir)/hicolor/48x48/apps/kradio_record.png
	$(INSTALL_DATA) $(srcdir)/kradio_record-64.png $(kde_icondir)/hicolor/64x64/apps/kradio_record.png
	$(INSTALL_DATA) $(srcdir)/kradio_kalarm_16.png $(kde_icondir)/hicolor/16x16/apps/kradio_kalarm.png
	$(INSTALL_DATA) $(srcdir)/kradio_kalarm_22.png $(kde_icondir)/hicolor/22x22/apps/kradio_kalarm.png
	$(INSTALL_DATA) $(srcdir)/kradio_kalarm_32.png $(kde_icondir)/hicolor/32x32/apps/kradio_kalarm.png
	$(INSTALL_DATA) $(srcdir)/kradio_kalarm_48.png $(kde_icondir)/hicolor/48x48/apps/kradio_kalarm.png
	$(INSTALL_DATA) $(srcdir)/kradio-zzz-16x16.png $(kde_icondir)/hicolor/16x16/apps/kradio-zzz.png
	$(INSTALL_DATA) $(srcdir)/kradio-zzz-22x22.png $(kde_icondir)/hicolor/22x22/apps/kradio-zzz.png
	$(INSTALL_DATA) $(srcdir)/kradio-zzz-32x32.png $(kde_icondir)/hicolor/32x32/apps/kradio-zzz.png
	$(INSTALL_DATA) $(srcdir)/kradio-zzz-48x48.png $(kde_icondir)/hicolor/48x48/apps/kradio-zzz.png

uninstall-local:
	-rm -f $(kde_appsdir)/Applications/kradio.desktop
	-rm -f $(kde_icondir)/locolor/16x16/apps/kradio.png
	-rm -f $(kde_icondir)/locolor/22x22/apps/kradio.png
	-rm -f $(kde_icondir)/locolor/24x24/apps/kradio.png
	-rm -f $(kde_icondir)/locolor/32x32/apps/kradio.png
	-rm -f $(kde_icondir)/locolor/48x48/apps/kradio.png
	-rm -f $(kde_icondir)/locolor/64x64/apps/kradio.png
	-rm -f $(kde_icondir)/hicolor/16x16/apps/kradio.png
	-rm -f $(kde_icondir)/hicolor/22x22/apps/kradio.png
	-rm -f $(kde_icondir)/hicolor/32x32/apps/kradio.png
	-rm -f $(kde_icondir)/hicolor/24x24/apps/kradio.png
	-rm -f $(kde_icondir)/hicolor/48x48/apps/kradio.png
	-rm -f $(kde_icondir)/hicolor/64x64/apps/kradio.png
	-rm -f $(kde_icondir)/hicolor/22x22/apps/kradio_muteoff.png
	-rm -f $(kde_icondir)/hicolor/22x22/apps/kradio_muteon.png
	-rm -f $(kde_icondir)/hicolor/16x16/apps/kradio_muteoff.png
	-rm -f $(kde_icondir)/hicolor/16x16/apps/kradio_muteon.png
	-rm -f $(kde_icondir)/hicolor/16x16/apps/kradio_record.png
	-rm -f $(kde_icondir)/hicolor/22x22/apps/kradio_record.png
	-rm -f $(kde_icondir)/hicolor/24x24/apps/kradio_record.png
	-rm -f $(kde_icondir)/hicolor/32x32/apps/kradio_record.png
	-rm -f $(kde_icondir)/hicolor/48x48/apps/kradio_record.png
	-rm -f $(kde_icondir)/hicolor/64x64/apps/kradio_record.png
	-rm -f $(kde_icondir)/hicolor/16x16/apps/kradio_kalarm.png
	-rm -f $(kde_icondir)/hicolor/22x22/apps/kradio_kalarm.png
	-rm -f $(kde_icondir)/hicolor/32x32/apps/kradio_kalarm.png
	-rm -f $(kde_icondir)/hicolor/48x48/apps/kradio_kalarm.png
	-rm -f $(kde_icondir)/hicolor/16x16/apps/kradio-zzz.png
	-rm -f $(kde_icondir)/hicolor/22x22/apps/kradio-zzz.png
	-rm -f $(kde_icondir)/hicolor/32x32/apps/kradio-zzz.png
	-rm -f $(kde_icondir)/hicolor/48x48/apps/kradio-zzz.png

####### kdevelop will overwrite this part!!! (end)############
# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to.
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

METASOURCES = AUTO

# the library search path.
kradio_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIRC)

# Uncomment the following two lines if you add a ui.rc file for your application to make use of
# KDE
rcdir = $(kde_datadir)/kradio
rc_DATA = kradioui.rc

#messages: rc.cpp
#	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
#	if test -n "$$LIST"; then \
#	  $(XGETTEXT) -C -ki18n -ktr2i18n -kI18N_NOOP $$LIST -o $(top_srcdir)/po/kradio.pot ; \
#	fi
messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kradio.pot

