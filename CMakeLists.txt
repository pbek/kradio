project(kradio5 CXX)

cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
cmake_policy(SET CMP0063 NEW)
cmake_policy(SET CMP0071 NEW)
cmake_policy(SET CMP0054 NEW)

ENABLE_LANGUAGE(C CXX)

# Locate extra-cmake-modules version 0.0.11 and make it a required package
find_package(ECM 5.10.0 REQUIRED NO_MODULE)

# Set value of CMAKE_MODULE_PATH variable where cmake will search for modules
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

# include KF5 directories
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)
include(ECMInstallIcons)

include (FindPkgConfig)
include (FindBoost)

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

SET(KRADIO_VERSION "5.0.0")

MESSAGE(STATUS "Compiling KRadio5-${KRADIO_VERSION}")



########################################################################
#       Check if libboost is available                                 #
########################################################################

find_package( Boost 1.55.0 REQUIRED)

########################################################################
#       KDE/QT Libs                                                    #
########################################################################

# Locate Qt5, make it required package and use QtWidgets and QtCore modules
find_package(Qt5 REQUIRED COMPONENTS Widgets Core Multimedia )

# Locate KF5, make it required package 
find_package(KF5 REQUIRED COMPONENTS  CoreAddons XmlGui I18n KIO WindowSystem WidgetsAddons KDELibs4Support )


########################################################################

find_package(Gettext)
if(NOT GETTEXT_MSGFMT_EXECUTABLE)
    message(STATUS "msgfmt not found. Translations will *not* be installed")
endif()
include(MacroProcessPoFiles)

########################################################################

ADD_DEFINITIONS (
    -DKRADIO_VERSION="${KRADIO_VERSION}"
    -DQT_STRICT_ITERATORS
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_USE_QSTRINGBUILDER
    -DQT_USE_FAST_OPERATOR_PLUS
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_TO_ASCII
    -Wall
    -Wextra    
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
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/radiostations
)

SET(INSTALL_TARGETS_DEFAULT_ARGS LIBRARY DESTINATION "${LIB_INSTALL_DIR}/kradio5/plugins")





ADD_SUBDIRECTORY(icons)
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


file(GLOB kradio5_UIs
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/radiostations/*.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/*.ui
)

ki18n_wrap_ui(kradio_main_sources
    ${kradio5_UIs}
)

add_executable(kradio5
    ${kradio_interfaces_sources}
    ${kradio_radiostation_sources}
    ${kradio_main_sources}
)

SET_TARGET_PROPERTIES(kradio5
    PROPERTIES
    LINK_FLAGS    " -Wl,--export-dynamic "
)

TARGET_LINK_LIBRARIES(kradio5
    Qt5::Core
    Qt5::Widgets
    Qt5::Multimedia
    KF5::XmlGui
    KF5::CoreAddons
    KF5::KIOWidgets
    KF5::KIOCore
    KF5::WidgetsAddons
    KF5::I18n
    KF5::WindowSystem
    KF5::KDELibs4Support
)

INSTALL(TARGETS kradio5)

SET(documentation_files
    README
    README.PVR
    README.your-help-is-wanted
    ChangeLog
    AUTHORS
    COPYING
    FAQ
)

INSTALL(
	FILES       ${documentation_files}
    DESTINATION ${SHARE_INSTALL_PREFIX}/doc/kradio5/
)

INSTALL(
    FILES       kradio5.desktop
    DESTINATION ${XDG_APPS_INSTALL_DIR}
)


feature_summary(WHAT ALL)

