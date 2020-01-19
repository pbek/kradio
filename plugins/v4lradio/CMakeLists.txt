project(kradio5_plugin_v4lradio)

# Locate extra-cmake-modules version 0.0.11 and make it a required package
find_package(ECM 5.10.0 REQUIRED NO_MODULE)

include(CheckIncludeFileCXX)
include(CheckLibraryExists)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)

########################################################################
#       KDE/QT Libs                                                    #
########################################################################

# Locate Qt5, make it required package and use QtWidgets and QtCore modules
find_package(Qt5 REQUIRED COMPONENTS Widgets Core Multimedia )

# Locate KF5, make it required package and use KF5KDELibs4Support and KF5CoreAddons modules
find_package(KF5 REQUIRED COMPONENTS  CoreAddons XmlGui I18n KIO)

###############################################################################

add_definitions( -DPROJECT_NAME="${PROJECT_NAME}" -DTRANSLATION_DOMAIN="${PROJECT_NAME}" )

ADD_SUBDIRECTORY(icons)
ADD_SUBDIRECTORY(po)



FILE(GLOB kradio_plugin_v4lradio_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

FILE(GLOB kradio_plugin_v4lradio_UIs
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)

ki18n_wrap_ui(kradio_plugin_v4lradio_sources
    ${kradio_plugin_v4lradio_UIs}
)


string(REGEX REPLACE "-Wl,--no-undefined" "" CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS})


add_library(kradio5_plugin_v4lradio MODULE
    ${kradio_plugin_v4lradio_sources}
)

TARGET_LINK_LIBRARIES(kradio5_plugin_v4lradio
    Qt5::Core
    Qt5::Widgets
    Qt5::Multimedia
    KF5::XmlGui
    KF5::CoreAddons
    KF5::KIOWidgets
    KF5::I18n
)

INSTALL(TARGETS kradio5_plugin_v4lradio ${INSTALL_TARGETS_DEFAULT_ARGS})

###############################################################################

option(WITH_V4L2 "Compile with V4L2 support" ON)

IF (WITH_V4L2)
    message(STATUS "   Compiling with V4L2 support")
    add_definitions( -DHAVE_V4L2 )
ELSE(WITH_V4L2)
    message(STATUS "   Compiling without V4L2 support")
ENDIF(WITH_V4L2)

add_feature_info("V4L2 Support"  WITH_V4L2 "Support for V4L2 support. Recommended and necessary for recent kernels > 2.6.30" "http://www.linuxtv.org/wiki/index.php/Main_Page")

###############################################################################


option(WITH_RDS_DEBUG_EMULATE "Do some RDS emulation for RDS debugging without an RDS-enabled card" OFF)

IF (WITH_RDS_DEBUG_EMULATE)
    message(STATUS "   Compiling with RDS-Debug-Emulation")
    add_definitions( -DRDS_DEBUG_EMULATE )
ELSE(WITH_RDS_DEBUG_EMULATE)
    message(STATUS "   Compiling without RDS-Debug-Emulation")
ENDIF(WITH_RDS_DEBUG_EMULATE)

#MACRO_LOG_FEATURE(WITH_RDS_DEBUG_EMULATE "RDS Debug Emulation"  "Emulate some dummy RDS message for testing RDS functionality without having an RDS renabled radio card" "http://www.linuxtv.org/wiki/index.php/Radio_Data_System_%28RDS%29")

###############################################################################

