project(kradio5_plugin_dbus)

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

ADD_SUBDIRECTORY(po)

FILE(GLOB kradio_plugin_dbus_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

FILE(GLOB kradio_plugin_dbus_UIs
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)

ki18n_wrap_ui(kradio_plugin_dbus_sources
    ${kradio_plugin_dbus_UIs}
)

QT5_ADD_DBUS_ADAPTOR(kradio_plugin_dbus_sources net.sourceforge.kradio.xml
                     dbussupport.h DBusSupport)


string(REGEX REPLACE "-Wl,--no-undefined" "" CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS})


add_library(kradio5_plugin_dbus MODULE
    ${kradio_plugin_dbus_sources}
)

TARGET_LINK_LIBRARIES(kradio5_plugin_dbus
    Qt5::Core
    Qt5::Widgets
    Qt5::Multimedia
    KF5::XmlGui
    KF5::CoreAddons
    KF5::KIOWidgets
    KF5::I18n
)

INSTALL(TARGETS kradio5_plugin_dbus ${INSTALL_TARGETS_DEFAULT_ARGS})


