project(kradio4)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

FIND_PACKAGE(Subversion)

SET(KRADIO_VERSION "4.0.7")

IF(Subversion_FOUND)
    execute_process(COMMAND svn info ${CMAKE_CURRENT_SOURCE_DIR}
                    COMMAND grep "^Revision: "
                    COMMAND gawk "{print $2}"
                    RESULT_VARIABLE SVN_CHECK_RESULT
                    OUTPUT_VARIABLE SVN_CHECK_STDOUT
                    ERROR_VARIABLE  SVN_CHECK_STDERR
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                   )

    IF( ${SVN_CHECK_RESULT} EQUAL 0  AND NOT  ${SVN_CHECK_STDOUT} EQUAL "" )
        MESSAGE(STATUS "Compiling KRadio4 from SVN revision ${SVN_CHECK_STDOUT}")
        SET(KRADIO_VERSION "svn-r${SVN_CHECK_STDOUT}")
    ENDIF( ${SVN_CHECK_RESULT} EQUAL 0  AND NOT  ${SVN_CHECK_STDOUT} EQUAL "" )
ENDIF(Subversion_FOUND)

MESSAGE(STATUS "Compiling KRadio4-${KRADIO_VERSION}")



########################################################################
#       Check if libboost is available                                 #
########################################################################


find_package( Boost 1.20 REQUIRED)
# find_package( Boost 1.xx COMPONENTS regex REQUIRED)



########################################################################

ADD_DEFINITIONS (
    -DKRADIO_VERSION="\\"${KRADIO_VERSION}\\""
    ${KDE4_DEFINITIONS}
)

option(WITH_KRADIO_FIXMES "Enable FIXME warnings" OFF)
IF(WITH_KRADIO_FIXMES)
    ADD_DEFINITIONS( -DKRADIO_ENABLE_FIXMES )
ENDIF(WITH_KRADIO_FIXMES)

LINK_DIRECTORIES(
    ${Boost_LIBRARY_DIRS}
)

INCLUDE_DIRECTORIES(
    ${KDE4_INCLUDE_DIR}
    ${QT_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/radiostations
)

SET(INSTALL_TARGETS_DEFAULT_ARGS RUNTIME DESTINATION "${BIN_INSTALL_DIR}"
                                 LIBRARY DESTINATION "${LIB_INSTALL_DIR}/kradio4/plugins"
)

KDE4_INSTALL_ICONS( ${ICON_INSTALL_DIR}  )





ADD_SUBDIRECTORY(icons)
#ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(plugins)
ADD_SUBDIRECTORY(presets)
ADD_SUBDIRECTORY(po)
ADD_SUBDIRECTORY(convert-presets)

########################################################################
#     The lines below only affect the executable, no plugins!          #
########################################################################

ADD_DEFINITIONS(
    -DQT3_SUPPORT
    -DKDE3_SUPPORT
    -DQT3_SUPPORT_WARNINGS
)


file(GLOB kradio_radiostation_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/radiostations/*.cpp
)

file(GLOB kradio_interfaces_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/*_interfaces.cpp
)

file(GLOB kradio_main_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)


file(GLOB kradio4_UIs
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/radiostations/*.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/*.ui
)

kde4_add_ui_files(kradio_main_sources
    ${kradio4_UIs}
)

kde4_add_executable(kradio4
    ${kradio_interfaces_sources}
    ${kradio_radiostation_sources}
    ${kradio_main_sources}
)

SET_TARGET_PROPERTIES(kradio4
    PROPERTIES
    LINK_FLAGS    " -Wl,--export-dynamic "
)

TARGET_LINK_LIBRARIES(kradio4
  ${KDE4_KDEUI_LIBS}
  ${QT_QT3SUPPORT_LIBRARY}
  ${KDE4_KDE3SUPPORT_LIBRARY}
  ${Boost_LIBRARIES}
)

INSTALL(TARGETS kradio4 ${INSTALL_TARGETS_DEFAULT_ARGS})

SET(documentation_files
    README
    README.PVR
    README.your-help-is-wanted
    README.translations
    ChangeLog
    TODO
    AUTHORS
    COPYING
    FAQ
    INSTALL
    REQUIREMENTS
)

INSTALL(
	FILES       ${documentation_files}
    DESTINATION ${SHARE_INSTALL_PREFIX}/doc/kradio4/
)

INSTALL(
    FILES       kradio4.desktop
    DESTINATION ${XDG_APPS_INSTALL_DIR}
)


MACRO_DISPLAY_FEATURE_LOG()

