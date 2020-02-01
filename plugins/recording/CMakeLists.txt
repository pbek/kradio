project(kradio5_plugin_recording)

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

include(CheckIncludeFile)

ADD_SUBDIRECTORY(icons)
ADD_SUBDIRECTORY(po)

option(WITH_OGG_VORBIS "Compile with ogg/vorbis support" ON)
option(WITH_LAME       "Compile with mp3/lame   support" ON)

###############################################################################

IF(WITH_OGG_VORBIS)
    pkg_check_modules(PKG_OGG       ogg)
    pkg_check_modules(PKG_VORBIS    vorbis)
    pkg_check_modules(PKG_VORBISENC vorbisenc)
    IF(NOT PKG_OGG_FOUND OR NOT PKG_VORBIS_FOUND OR NOT PKG_VORBISENC_FOUND)

        SET(WITH_OGG_VORBIS OFF
            CACHE BOOL "ogg/vorbis files could not be found on this system. Forcing this option to OFF"
            FORCE)
        MESSAGE("WARNING: ogg/vorbis files could not be found on this system. Compiling without ogg/vorbis support")

    ENDIF(NOT PKG_OGG_FOUND OR NOT PKG_VORBIS_FOUND OR NOT PKG_VORBISENC_FOUND)
ENDIF(WITH_OGG_VORBIS)


IF (WITH_OGG_VORBIS)
    message(STATUS "   xCompiling with ogg/vorbis support")
    add_definitions( -DHAVE_OGG )
    include_directories(${PKG_VORBISENC_INCLUDE_DIRS} ${PKG_VORBIS_INCLUDE_DIRS} ${PKG_OGG_INCLUDE_DIRS})
    set(LIB_OGG ${PKG_OGG_LIBRARY_DIRS} ${PKG_VORBISENC_LIBRARY_DIRS} ${PKG_VORBIS_LIBRARY_DIRS})
ELSE(WITH_OGG_VORBIS)
    message(STATUS "   xCompiling without ogg/vorbis support")
    set(LIB_OGG "")
ENDIF(WITH_OGG_VORBIS)

add_feature_info("ogg/vorbis support"  WITH_OGG_VORBIS "Support for audio compression format ogg/vorbis" "http://www.vorbis.com") 

###############################################################################


IF(WITH_LAME)
    find_package(Lame)
    IF(NOT Lame_FOUND)

        SET(WITH_LAME OFF
            CACHE BOOL "mp3/lame files could not be found on this system. Forcing this option to OFF"
            FORCE)
        MESSAGE("WARNING: mp3/lame files could not be found on this system. Compiling without mp3/lame support")

    ENDIF(NOT Lame_FOUND)
ENDIF(WITH_LAME)


IF (WITH_LAME)
    message(STATUS "   Compiling with mp3/lame support")
    add_definitions( -DHAVE_LAME )
    include_directories(${Lame_INCLUDE_DIRS})
    set(LIB_LAME ${Lame_LIBRARIES})
ELSE(WITH_LAME)
    message(STATUS "   Compiling without mp3/lame support")
    set(LIB_LAME "")
ENDIF(WITH_LAME)

add_feature_info("mp3/lame support"  WITH_LAME "Support for audio compression format .mp3 via lame" "http://lame.sourceforge.net") 

###############################################################################

SET(WITH_LIBSNDFILE ON)

find_package(Sndfile REQUIRED)
SET(WITH_LIBSNDFILE ${Sndfile_FOUND})

include_directories(${Sndfile_INCLUDE_DIRS})

add_feature_info("libsndfile generic sound file support"  WITH_LIBSNDFILE "Support for some standard audio formats (e.g. .wav)" "http://www.mega-nerd.com/libsndfile" TRUE)

###############################################################################


FILE(GLOB kradio_plugin_recording_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

FILE(GLOB kradio_plugin_recording_UIs
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)

ki18n_wrap_ui(kradio_plugin_recording_sources
    ${kradio_plugin_recording_UIs}
)


string(REGEX REPLACE "-Wl,--no-undefined" "" CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS})


add_library(kradio5_plugin_recording MODULE
    ${kradio_plugin_recording_sources}
)

TARGET_LINK_LIBRARIES(kradio5_plugin_recording
    Qt5::Core
    Qt5::Widgets
    Qt5::Multimedia
    KF5::XmlGui
    KF5::CoreAddons
    KF5::KIOWidgets
    KF5::I18n
    ${LIB_OGG}
    ${LIB_LAME}
    ${Sndfile_LIBRARIES}
)

SET(CMAKE_INSTALL_RPATH                              "${LIB_INSTALL_DIR}/kradio5/plugins")
SET(INSTALL_TARGETS_DEFAULT_ARGS LIBRARY DESTINATION "${CMAKE_INSTALL_RPATH}")
INSTALL(TARGETS kradio5_plugin_recording ${INSTALL_TARGETS_DEFAULT_ARGS})

