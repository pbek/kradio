project(kradio5_plugin_internetradio)

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

#add_definitions( -DINET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD )


#ADD_SUBDIRECTORY(icons)
ADD_SUBDIRECTORY(po)

add_definitions( -D__STDC_CONSTANT_MACROS )

IF(SWRESAMPLE_FOUND)
    add_definitions( -DHAVE_LIBSWRESAMPLE )
ENDIF(SWRESAMPLE_FOUND)

IF(AVRESAMPLE_FOUND)
    add_definitions( -DHAVE_LIBAVRESAMPLE )
ENDIF(AVRESAMPLE_FOUND)

INCLUDE_DIRECTORIES(
    ${FFMPEG_INCLUDE_DIRS}
    ${Libmms_INCLUDE_DIRS}
)

FILE(GLOB kradio_plugin_internetradio_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

FILE(GLOB kradio_plugin_internetradio_UIs
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)

ki18n_wrap_ui(kradio_plugin_internetradio_sources
    ${kradio_plugin_internetradio_UIs}
)


string(REGEX REPLACE "-Wl,--no-undefined" "" CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS})


add_library(kradio5_plugin_internetradio MODULE
    ${kradio_plugin_internetradio_sources}
)

TARGET_LINK_LIBRARIES(kradio5_plugin_internetradio
    Qt5::Core
    Qt5::Widgets
    Qt5::Multimedia
    KF5::XmlGui
    KF5::CoreAddons
    KF5::KIOWidgets
    KF5::I18n
    ${FFMPEG_LIBRARIES}
    ${LIBSWRESAMPLE_LIBRARIES}
    ${Libmms_LIBRARIES}
)

SET(CMAKE_INSTALL_RPATH                              "${LIB_INSTALL_DIR}/kradio5/plugins")
SET(INSTALL_TARGETS_DEFAULT_ARGS LIBRARY DESTINATION "${CMAKE_INSTALL_RPATH}")
INSTALL(TARGETS kradio5_plugin_internetradio ${INSTALL_TARGETS_DEFAULT_ARGS})
