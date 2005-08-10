INCLUDES = $(all_includes)
METASOURCES = AUTO

libkradio_LTLIBRARIES = libtimeshifter.la
libtimeshifter_la_SOURCES = timeshifter.cpp timeshifter-configuration-ui.ui \
                            timeshifter-configuration.cpp
libtimeshifter_la_LDFLAGS = -module -avoid-version $(KDE_RPATH) $(all_libraries)

noinst_HEADERS = timeshifter.h timeshifter-configuration.h
