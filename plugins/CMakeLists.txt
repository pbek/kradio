project(kradio4plugins)

# search packages used by KDE
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (FindPkgConfig)
include (MacroLibrary)

include(CheckIncludeFile)
include(CheckIncludeFileCXX)

###############################################################################

option(WITH_ALSA "Compile with alsa support" ON)

IF(WITH_ALSA)
    SET (ALSA_LIBRARY_DIRS  "")
    SET (ALSA_INCLUDE_DIRS  "")
    SET (ALSA_LIBRARIES     "")

    pkg_check_modules(LIBASOUND alsa)
    IF(LIBASOUND_FOUND)
        SET(HAVE_AVFORMAT_INC ON)
        SET(HAVE_AVFORMAT_LIB ON)

        SET(ALSA_LIBRARY_DIRS ${ALSA_LIBRARY_DIRS} ${LIBASOUND_LIBRARY_DIRS})
        SET(ALSA_LIBRARIES    ${ALSA_LIBRARIES}    ${LIBASOUND_LIBRARIES}   )
        SET(ALSA_INCLUDE_DIRS ${ALSA_INCLUDE_DIRS} ${LIBASOUND_INCLUDE_DIRS})

    ELSE(LIBASOUND_FOUND)
        CHECK_INCLUDE_FILE_CXX(alsa/asoundlib.h      HAVE_ALSA_INC)
        CHECK_LIBRARY_EXISTS(asound snd_card_next "" HAVE_ALSA_LIB)
        IF (HAVE_ALSA_INC AND HAVE_ALSA_LIB)
            SET(LIBASOUND_FOUND ON)
            SET(ALSA_LIBRARIES    "asound" ${ALSA_LIBRARIES})
        ENDIF(HAVE_ALSA_INC AND HAVE_ALSA_LIB)
    ENDIF(LIBASOUND_FOUND)

    IF(NOT LIBASOUND_FOUND)

        SET(WITH_ALSA OFF
            CACHE BOOL "alsa files could not be found on this system. Forcing this option to OFF"
            FORCE)
        MESSAGE("WARNING: alsa files could not be found on this system. Compiling without alsa support")

    ENDIF(NOT LIBASOUND_FOUND)
ENDIF(WITH_ALSA)


IF (WITH_ALSA)
    message(STATUS "   Compiling with ALSA support")
    message(STATUS "       ALSA_INCLUDE_DIRS = ${ALSA_INCLUDE_DIRS}")
    message(STATUS "       ALSA_LIBRARY_DIRS = ${ALSA_LIBRARY_DIRS}")
    message(STATUS "       ALSA_LIBRARIES    = ${ALSA_LIBRARIES}")
    message(STATUS "   Compiling with alsa support")
    add_definitions( -DHAVE_ALSA )
ELSE(WITH_ALSA)
    message(STATUS "   Compiling without ALSA support")
ENDIF(WITH_ALSA)


MACRO_LOG_FEATURE(WITH_ALSA "ALSA support" "Support for ALSA Sound Devices" "http://www.alsa-project.org")

###############################################################################


option(WITH_LIRC "Compile with LIRC support" ON)

IF(WITH_LIRC)
    CHECK_INCLUDE_FILE_CXX(lirc/lirc_client.h HAVE_LIRC_INC)
    CHECK_LIBRARY_EXISTS(lirc_client lirc_init "" HAVE_LIRC_LIB)
    IF(NOT HAVE_LIRC_INC OR NOT HAVE_LIRC_LIB)

        SET(WITH_LIRC OFF
            CACHE BOOL "LIRC files could not be found on this system. Forcing this option to OFF"
            FORCE)
        MESSAGE("WARNING: LIRC files could not be found on this system. Compiling without LIRC support")

    ENDIF(NOT HAVE_LIRC_INC OR NOT HAVE_LIRC_LIB)
ENDIF(WITH_LIRC)


IF (WITH_LIRC)
    message(STATUS "   Compiling with LIRC support")
    add_definitions( -DHAVE_LIRC )
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
    IF(LIBAVFORMAT_FOUND)
        SET(HAVE_AVFORMAT_INC ON)
        SET(HAVE_AVFORMAT_LIB ON)

        SET(FFMPEG_LIBRARY_DIRS ${FFMPEG_LIBRARY_DIRS} ${LIBAVFORMAT_LIBRARY_DIRS} ${LIBAVCODEC_LIBRARY_DIRS} )
        SET(FFMPEG_LIBRARIES    ${FFMPEG_LIBRARIES}    ${LIBAVFORMAT_LIBRARIES}    ${LIBAVCODEC_LIBRARIES}    )
        SET(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS} ${LIBAVFORMAT_INCLUDE_DIRS} ${LIBAVCODEC_INCLUDE_DIRS} )

    ELSE(LIBAVFORMAT_FOUND)
        CHECK_INCLUDE_FILE_CXX(ffmpeg/avformat.h            HAVE_AVFORMAT_OLD_INC)
        CHECK_INCLUDE_FILE_CXX(libavformat/avformat.h       HAVE_AVFORMAT_INC)
        CHECK_LIBRARY_EXISTS(avformat av_open_input_file "" HAVE_AVFORMAT_LIB)
        IF (HAVE_AVFORMAT_OLD_INC OR HAVE_AVFORMAT_INC)
            IF (HAVE_AVFORMAT_LIB)
                SET(LIBAVFORMAT_FOUND ON)
                SET(FFMPEG_LIBRARIES    "avutil" "avformat" ${FFMPEG_LIBRARIES})
            ENDIF(HAVE_AVFORMAT_LIB)
        ENDIF(HAVE_AVFORMAT_OLD_INC OR HAVE_AVFORMAT_INC)
    ENDIF(LIBAVFORMAT_FOUND)



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


    pkg_check_modules(LIBMMS libmms)
    IF(LIBMMS_FOUND)
        SET(LIBMMS_MIN_VERSION 0.5)
        IF(NOT LIBMMS_VERSION LESS LIBMMS_MIN_VERSION)
            SET(HAVE_MMSX_INC ON)
            SET(HAVE_MMSX_LIB ON)

            SET(FFMPEG_LIBRARY_DIRS ${FFMPEG_LIBRARY_DIRS} ${LIBMMS_LIBRARY_DIRS})
            SET(FFMPEG_LIBRARIES    ${FFMPEG_LIBRARIES}    ${LIBMMS_LIBRARIES}   )
            SET(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS} ${LIBMMS_INCLUDE_DIRS})
        ELSE(NOT LIBMMS_VERSION LESS LIBMMS_MIN_VERSION)
            MESSAGE("WARNING: found libmms " ${LIBMMS_VERSION} ", however, version >= " LIBMMS_MIN_VERSION " required")
            SET(LIBMMS_FOUND false)
        ENDIF(NOT LIBMMS_VERSION LESS LIBMMS_MIN_VERSION)
    ELSE(LIBMMS_FOUND)
        CHECK_INCLUDE_FILE("libmms/mmsx.h"       HAVE_MMSX_INC)
        CHECK_LIBRARY_EXISTS(mms mmsx_connect "" HAVE_MMSX_LIB)
        IF (HAVE_MMSX_INC AND HAVE_MMSX_LIB)
            SET(LIBMMS_FOUND ON)
            SET(FFMPEG_LIBRARIES "mms" ${FFMPEG_LIBRARIES})
        ENDIF(HAVE_MMSX_INC AND HAVE_MMSX_LIB)
    ENDIF(LIBMMS_FOUND)

    IF ( NOT LIBMMS_FOUND OR NOT LIBAVFORMAT_FOUND )
        SET(WITH_FFMPEG OFF
            CACHE BOOL "libffmpeg and/or libmms files could not be found on this system. Forcing this option to OFF"
            FORCE)
        MESSAGE("WARNING: libffmpeg and/or libmms files could not be found on this system. Compiling without Internet Radio Stations support")
    ENDIF( NOT LIBMMS_FOUND OR NOT LIBAVFORMAT_FOUND )

ENDIF(WITH_FFMPEG)


IF (WITH_FFMPEG)
    message(STATUS "   Compiling with Internet Radio Stations (libavformat/libffmpeg, libmms) support")
    message(STATUS "       FFMPEG_INCLUDE_DIRS = ${FFMPEG_INCLUDE_DIRS}")
    message(STATUS "       FFMPEG_LIBRARY_DIRS = ${FFMPEG_LIBRARY_DIRS}")
    message(STATUS "       FFMPEG_LIBRARIES    = ${FFMPEG_LIBRARIES}")

    IF(HAVE_AVFORMAT_OLD_INC)
        add_definitions( -DHAVE_FFMPEG_OLD )
    ELSE(HAVE_AVFORMAT_OLD_INC)
        add_definitions( -DHAVE_FFMPEG -D__STDC_CONSTANT_MACROS )
    ENDIF(HAVE_AVFORMAT_OLD_INC)

    add_definitions( -DHAVE_MMS )
ELSE(WITH_FFMPEG)
    message(STATUS "   Compiling without Internet Radio Stations (libffmpeg, libmms) support")
ENDIF(WITH_FFMPEG)

MACRO_LOG_FEATURE(LIBAVFORMAT_FOUND "FFMPEG support"          "Support for internet radio by libffmpeg" "http://ffmpeg.org")
MACRO_LOG_FEATURE(LIBMMS_FOUND      "MMS protocol support"    "Support internet radio protocol mms:// by libmms >= ${LIBMMS_MIN_VERSION}"  "https://launchpad.net/libmms")
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
