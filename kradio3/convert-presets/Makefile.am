subdirs = po .

bin_PROGRAMS = convert-presets

convert_presets_SOURCES = convert-presets.cpp
convert_presets_LDADD  = $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE)


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
convert_presets_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# messages: rc.cpp
messages: rc.cpp
	$(XGETTEXT) *.cpp -o po/kradio-convert-presets.pot

#messages:
#	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
#	if test -n "$$LIST"; then \
#	  $(XGETTEXT) -C -ki18n -ktr2i18n $$LIST -o $(top_srcdir)/po/kradio.pot ; \
#	fi

