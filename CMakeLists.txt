project(kradio4)

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(SET CMP0059 OLD)
cmake_policy(SET CMP0064 OLD)

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(KDE4 4.7.0 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

SET(KRADIO_VERSION "4.0.8")

MESSAGE(STATUS "Compiling KRadio4-${KRADIO_VERSION}")



########################################################################
#       Check if libboost is available                                 #
########################################################################


find_package( Boost 1.20 REQUIRED)
# find_package( Boost 1.xx COMPONENTS regex REQUIRED)



########################################################################

find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt)
if(NOT GETTEXT_MSGFMT_EXECUTABLE)
    message(STATUS "msgfmt not found. Translations will *not* be installed")
endif()
include(MacroProcessPoFiles)

########################################################################

ADD_DEFINITIONS (
    -DKRADIO_VERSION="${KRADIO_VERSION}"
    ${KDE4_DEFINITIONS}
    -DQT_STRICT_ITERATORS
)

option(WITH_KRADIO_FIXMES "Enable FIXME warnings" OFF)
IF(WITH_KRADIO_FIXMES)
    ADD_DEFINITIONS( -DKRADIO_ENABLE_FIXMES )
ENDIF(WITH_KRADIO_FIXMES)

option(WITH_KRADIO_PROFILERS "Enable the runtime profilers" OFF)
if(WITH_KRADIO_PROFILERS)
    add_definitions(-DKRADIO_ENABLE_PROFILERS)
endif()

INCLUDE_DIRECTORIES(
    ${KDE4_INCLUDE_DIR}
    ${QT_INCLUDES}
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/radiostations
)

SET(INSTALL_TARGETS_DEFAULT_ARGS RUNTIME DESTINATION "${BIN_INSTALL_DIR}"
                                 LIBRARY DESTINATION "${LIB_INSTALL_DIR}/kradio4/plugins"
)





ADD_SUBDIRECTORY(icons)
#ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(plugins)
ADD_SUBDIRECTORY(presets)
ADD_SUBDIRECTORY(po)
ADD_SUBDIRECTORY(convert-presets)

########################################################################
#     The lines below only affect the executable, no plugins!          #
########################################################################

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
  ${QT_QTXML_LIBRARY}
  ${KDE4_KIO_LIBRARY}
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

