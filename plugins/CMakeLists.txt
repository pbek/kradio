project(kradio4plugins)

find_package(PkgConfig)

include(CheckIncludeFileCXX)
include(CheckLibraryExists)

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


MACRO_LOG_FEATURE(WITH_ALSA "ALSA support" "Support for ALSA Sound Devices" "http://www.alsa-project.org")

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

MACRO_LOG_FEATURE(WITH_LIRC "LIRC support" "Support for remote controls via LIRC interfaces" "http://www.lirc.org")

###############################################################################




option(WITH_FFMPEG "Compile with Internet Radio Stations support (libavformat/libffmpeg, libmms)" ON)

IF(WITH_FFMPEG)
    SET (FFMPEG_LIBRARY_DIRS  "")
    SET (FFMPEG_INCLUDE_DIRS  "")
    SET (FFMPEG_LIBRARIES     "")

    pkg_check_modules(LIBAVFORMAT libavformat)
    pkg_check_modules(LIBAVCODEC  libavcodec)

    SET(FFMPEG_LIBRARY_DIRS ${FFMPEG_LIBRARY_DIRS} ${LIBAVFORMAT_LIBRARY_DIRS} ${LIBAVCODEC_LIBRARY_DIRS} )
    SET(FFMPEG_LIBRARIES    ${FFMPEG_LIBRARIES}    ${LIBAVFORMAT_LIBRARIES}    ${LIBAVCODEC_LIBRARIES}    )
    SET(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS} ${LIBAVFORMAT_INCLUDE_DIRS} ${LIBAVCODEC_INCLUDE_DIRS} )



    pkg_check_modules(LIBSWRESAMPLE libswresample)
    IF(LIBSWRESAMPLE_FOUND)
        SET(FFMPEG_LIBRARIES    ${FFMPEG_LIBRARIES}    ${LIBSWRESAMPLE_LIBRARIES}   )
        add_definitions( -DHAVE_LIBSWRESAMPLE )
    ENDIF(LIBSWRESAMPLE_FOUND)


    pkg_check_modules(LIBAVRESAMPLE libavresample)
    IF(LIBAVRESAMPLE_FOUND)
        SET(FFMPEG_LIBRARIES    ${FFMPEG_LIBRARIES}    ${LIBAVRESAMPLE_LIBRARIES}   )
        add_definitions( -DHAVE_LIBAVRESAMPLE )
    ENDIF(LIBAVRESAMPLE_FOUND)

    IF(NOT LIBAVCODEC_VERSION LESS 53.42.0)
        IF(NOT LIBSWRESAMPLE_FOUND AND NOT LIBAVRESAMPLE_FOUND)
            ERROR(FATAL_ERROR "ERROR: found libavcodec " ${LIBAVCODEC_VERSION} " which requires libswresample or libavesample. However, none is not available.")
        ENDIF(NOT LIBSWRESAMPLE_FOUND AND NOT LIBAVRESAMPLE_FOUND)
    ENDIF(NOT LIBAVCODEC_VERSION LESS 53.42.0)


    set(LIBMMS_MIN_VERSION 0.5)
    find_package(Libmms ${LIBMMS_MIN_VERSION})

    IF ( NOT Libmms_FOUND OR NOT LIBAVFORMAT_FOUND )
        SET(WITH_FFMPEG OFF
            CACHE BOOL "libffmpeg and/or libmms files could not be found on this system. Forcing this option to OFF"
            FORCE)
        MESSAGE("WARNING: libffmpeg and/or libmms files could not be found on this system. Compiling without Internet Radio Stations support")
    ENDIF( NOT Libmms_FOUND OR NOT LIBAVFORMAT_FOUND )

ENDIF(WITH_FFMPEG)


IF (WITH_FFMPEG)
    message(STATUS "   Compiling with Internet Radio Stations (libavformat/libffmpeg, libmms) support")
    message(STATUS "       FFMPEG_INCLUDE_DIRS = ${FFMPEG_INCLUDE_DIRS}")
    message(STATUS "       FFMPEG_LIBRARY_DIRS = ${FFMPEG_LIBRARY_DIRS}")
    message(STATUS "       FFMPEG_LIBRARIES    = ${FFMPEG_LIBRARIES}")
    message(STATUS "       Libmms_INCLUDE_DIRS = ${Libmms_INCLUDE_DIRS}")
    message(STATUS "       Libmms_LIBRARIES    = ${Libmms_LIBRARIES}")

    add_definitions( -D__STDC_CONSTANT_MACROS )
ELSE(WITH_FFMPEG)
    message(STATUS "   Compiling without Internet Radio Stations (libffmpeg, libmms) support")
ENDIF(WITH_FFMPEG)

MACRO_LOG_FEATURE(LIBAVFORMAT_FOUND "FFMPEG support"          "Support for internet radio by libffmpeg" "http://ffmpeg.org" FALSE "0.5.0")
MACRO_LOG_FEATURE(LIBMMS_FOUND      "MMS protocol support"    "Support internet radio protocol mms:// by libmms"  "https://launchpad.net/libmms" FALSE "${LIBMMS_MIN_VERSION}")
MACRO_LOG_FEATURE(WITH_FFMPEG       "Internet radio support"  "Internet radio requires both libffmpeg and libmms" "")

###############################################################################



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
