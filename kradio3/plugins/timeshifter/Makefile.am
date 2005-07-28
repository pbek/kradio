INCLUDES = $(all_includes)
METASOURCES = AUTO

lib_LTLIBRARIES = libtimeshifter.la
libtimeshifter_la_SOURCES = timeshifter.cpp timeshifter-configuration-ui.ui \
                            timeshifter-configuration.cpp
libtimeshifter_la_LDFLAGS = -module -avoid-version $(all_libraries)
libtimeshifter_la_LIBADD = ../../src/libkradio/libkradio.la

noinst_HEADERS = timeshifter.h timeshifter-configuration.h
libdir = $(prefix)/lib/kradio/plugins
