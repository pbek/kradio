INCLUDES = $(all_includes)
METASOURCES = AUTO

lib_LTLIBRARIES = libtimecontrol.la
libtimecontrol_la_SOURCES = timecontrol-configuration.cpp \
                            timecontrol-configuration-ui.ui timecontrol.cpp
libtimecontrol_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)
noinst_HEADERS =  timecontrol-configuration.h timecontrol.h
