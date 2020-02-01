project(kradio5plugins)

find_package(PkgConfig)

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

option(WITH_ALSA "Compile with alsa support" ON)

IF(WITH_ALSA)
    find_package(ALSA)

    IF(NOT ALSA_FOUND)

        SET(WITH_ALSA OFF
            CACHE BOOL "alsa files could not be found on this system. Forcing this option to OFF"
            FORCE)
        MESSAGE("WARNING: alsa files could not be found on this system. Compiling without alsa support")

    ENDIF(NOT ALSA_FOUND)
ENDIF(WITH_ALSA)


IF (WITH_ALSA)
    message(STATUS "   Compiling with ALSA support")
    message(STATUS "       ALSA_INCLUDE_DIRS = ${ALSA_INCLUDE_DIRS}")
    message(STATUS "       ALSA_LIBRARIES    = ${ALSA_LIBRARIES}")
    message(STATUS "   Compiling with alsa support")
ELSE(WITH_ALSA)
    message(STATUS "   Compiling without ALSA support")
ENDIF(WITH_ALSA)



add_feature_info("Alsa Support" WITH_ALSA "Support for ALSA Sound Devices" "http://www.alsa-project.org")

###############################################################################


option(WITH_LIRC "Compile with LIRC support" ON)

IF(WITH_LIRC)
    find_package(LircClient)

    IF(NOT LircClient_FOUND)

        SET(WITH_LIRC OFF
            CACHE BOOL "LIRC files could not be found on this system. Forcing this option to OFF"
            FORCE)
        MESSAGE("WARNING: LIRC files could not be found on this system. Compiling without LIRC support")

    ENDIF(NOT LircClient_FOUND)
ENDIF(WITH_LIRC)


IF (WITH_LIRC)
    message(STATUS "   Compiling with LIRC support")
ELSE(WITH_LIRC)
    message(STATUS "   Compiling without LIRC support")
ENDIF(WITH_LIRC)

add_feature_info("LIRC support" WITH_LIRC "Support for remote controls via LIRC interfaces" "http://www.lirc.org")

###############################################################################




option(WITH_FFMPEG "Compile with Internet Radio Stations support (libavformat/libffmpeg, libmms)" ON)

IF(WITH_FFMPEG)
    set(ffmpeg_components AVFORMAT AVCODEC)
    find_package(FFmpeg COMPONENTS ${ffmpeg_components} SWRESAMPLE)
    IF(NOT FFMPEG_FOUND)
        unset(FFMPEG_LIBRARIES CACHE)
        unset(FFMPEG_INCLUDE_DIRS CACHE)
        unset(FFMPEG_DEFINITIONS CACHE)
        find_package(FFmpeg COMPONENTS ${ffmpeg_components} AVRESAMPLE)
    ENDIF(NOT FFMPEG_FOUND)

    IF(AVFORMAT_VERSION VERSION_LESS 58.12.100)
       ERROR(FATAL_ERROR "libavformat ${AVFORMAT_FOUND} found, 58.12.100 required")
    ENDIF(AVFORMAT_VERSION VERSION_LESS 58.12.100)
    IF(AVCODEC_VERSION VERSION_LESS 58.18.100)
       ERROR(FATAL_ERROR "libavcodec ${AVCODEC_VERSION} found, 58.18.100 required")
    ENDIF(AVCODEC_VERSION VERSION_LESS 58.18.100)


    set(LIBMMS_MIN_VERSION 0.5)
    find_package(Libmms ${LIBMMS_MIN_VERSION})

    IF ( NOT Libmms_FOUND OR NOT FFMPEG_FOUND )
        SET(WITH_FFMPEG OFF
            CACHE BOOL "libffmpeg and/or libmms files could not be found on this system. Forcing this option to OFF"
            FORCE)
        MESSAGE("WARNING: libffmpeg and/or libmms files could not be found on this system. Compiling without Internet Radio Stations support")
    ENDIF( NOT Libmms_FOUND OR NOT FFMPEG_FOUND )

ENDIF(WITH_FFMPEG)


IF (WITH_FFMPEG)
    message(STATUS "   Compiling with Internet Radio Stations (libavformat/libffmpeg, libmms) support")
    message(STATUS "       FFMPEG_INCLUDE_DIRS = ${FFMPEG_INCLUDE_DIRS}")
    message(STATUS "       FFMPEG_LIBRARIES    = ${FFMPEG_LIBRARIES}")
    message(STATUS "       Libmms_INCLUDE_DIRS = ${Libmms_INCLUDE_DIRS}")
    message(STATUS "       Libmms_LIBRARIES    = ${Libmms_LIBRARIES}")
ELSE(WITH_FFMPEG)
    message(STATUS "   Compiling without Internet Radio Stations (libffmpeg, libmms) support")
ENDIF(WITH_FFMPEG)

add_feature_info("FFMPEG support"          FFMPEG_FOUND      "Support for internet radio by libffmpeg" "http://ffmpeg.org" FALSE "0.9.0")
add_feature_info("MMS protocol support"    LIBMMS_FOUND      "Support internet radio protocol mms:// by libmms"  "https://launchpad.net/libmms" FALSE "${LIBMMS_MIN_VERSION}")
add_feature_info("Internet radio support"  WITH_FFMPEG       "Internet radio requires both libffmpeg and libmms" "")

###############################################################################


feature_summary(WHAT ALL)

ADD_SUBDIRECTORY(radio)
IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    ADD_SUBDIRECTORY(v4lradio)
ENDIF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
IF(WITH_ALSA)
    ADD_SUBDIRECTORY(alsa-sound)
ENDIF(WITH_ALSA)
ADD_SUBDIRECTORY(soundserver)
ADD_SUBDIRECTORY(gui-docking-menu)
ADD_SUBDIRECTORY(timecontrol)
ADD_SUBDIRECTORY(timeshifter)
ADD_SUBDIRECTORY(streaming)
ADD_SUBDIRECTORY(recording)
ADD_SUBDIRECTORY(gui-quickbar)
ADD_SUBDIRECTORY(gui-error-log)
IF(WITH_LIRC)
    ADD_SUBDIRECTORY(lirc)
ENDIF(WITH_LIRC)
ADD_SUBDIRECTORY(gui-standard-display)
ADD_SUBDIRECTORY(shortcuts)
IF(WITH_FFMPEG)
    ADD_SUBDIRECTORY(internetradio)
ENDIF(WITH_FFMPEG)
ADD_SUBDIRECTORY(dbus)
ADD_SUBDIRECTORY(mpris)
