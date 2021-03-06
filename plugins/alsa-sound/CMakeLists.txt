project(kradio5_plugin_alsa_sound)

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

INCLUDE_DIRECTORIES(
    ${ALSA_INCLUDE_DIRS}
)



FILE(GLOB kradio_plugin_alsa_sound_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

FILE(GLOB kradio_plugin_alsa_sound_UIs
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)

ki18n_wrap_ui(kradio_plugin_alsa_sound_sources
    ${kradio_plugin_alsa_sound_UIs}
)


string(REGEX REPLACE "-Wl,--no-undefined" "" CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS})


add_library(kradio5_plugin_alsa_sound MODULE
    ${kradio_plugin_alsa_sound_sources}
)

TARGET_LINK_LIBRARIES(kradio5_plugin_alsa_sound
    Qt5::Core
    Qt5::Widgets
    Qt5::Multimedia
    KF5::XmlGui
    KF5::CoreAddons
    KF5::KIOWidgets
    KF5::I18n
    ${ALSA_LIBRARIES}
)

SET(CMAKE_INSTALL_RPATH                              "${LIB_INSTALL_DIR}/kradio5/plugins")
SET(INSTALL_TARGETS_DEFAULT_ARGS LIBRARY DESTINATION "${CMAKE_INSTALL_RPATH}")
INSTALL(TARGETS kradio5_plugin_alsa_sound ${INSTALL_TARGETS_DEFAULT_ARGS})
