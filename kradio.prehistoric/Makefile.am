####### kdevelop will overwrite this part!!! (begin)##########

SUBDIRS = kradio po presets convert-presets 

EXTRA_DIST = kradio.kdevprj AUTHORS COPYING ChangeLog INSTALL README TODO kradio.lsm kradio.spec 

install-data-local:
	$(mkinstalldirs) $(datadir)/doc/kradio/
	$(INSTALL_DATA) $(srcdir)/AUTHORS $(datadir)/doc/kradio/AUTHORS
	$(mkinstalldirs) $(datadir)/doc/kradio/
	$(INSTALL_DATA) $(srcdir)/COPYING $(datadir)/doc/kradio/COPYING
	$(mkinstalldirs) $(datadir)/doc/kradio/
	$(INSTALL_DATA) $(srcdir)/ChangeLog $(datadir)/doc/kradio/ChangeLog
	$(mkinstalldirs) $(datadir)/doc/kradio/
	$(INSTALL_DATA) $(srcdir)/INSTALL $(datadir)/doc/kradio/INSTALL
	$(mkinstalldirs) $(datadir)/doc/kradio/
	$(INSTALL_DATA) $(srcdir)/README $(datadir)/doc/kradio/README
	$(mkinstalldirs) $(datadir)/doc/kradio/
	$(INSTALL_DATA) $(srcdir)/TODO $(datadir)/doc/kradio/TODO

uninstall-local:
	-rm -f $(datadir)/doc/kradio/AUTHORS
	-rm -f $(datadir)/doc/kradio/COPYING
	-rm -f $(datadir)/doc/kradio/ChangeLog
	-rm -f $(datadir)/doc/kradio/INSTALL
	-rm -f $(datadir)/doc/kradio/README
	-rm -f $(datadir)/doc/kradio/TODO

####### kdevelop will overwrite this part!!! (end)############
# not a GNU package. You can remove this line, if 
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 

$(top_srcdir)/configure.in: configure.in.in $(top_srcdir)/subdirs
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common configure.in ;

$(top_srcdir)/subdirs:
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common subdirs

$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in
	@cd $(top_srcdir) && cat admin/acinclude.m4.in admin/libtool.m4.in > acinclude.m4

MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 

package-messages:
	$(MAKE) -f admin/Makefile.common package-messages
	$(MAKE) -C po merge

dist-hook:
	cd $(top_distdir) && perl admin/am_edit -padmin
	cd $(top_distdir) && $(MAKE) -f admin/Makefile.common subdirs
