project(kradio4plugins)

# search packages used by KDE
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (FindPkgConfig)
include (MacroLibrary)

include(CheckIncludeFile)
include(CheckIncludeFileCXX)

CHECK_INCLUDE_FILE(blah.h HAVE_BLAH_H)

###############################################################################

option(WITH_ALSA "Compile with alsa support" ON)

IF(WITH_ALSA)
    CHECK_INCLUDE_FILE_CXX(alsa/asoundlib.h HAVE_ALSA_INC)
    CHECK_LIBRARY_EXISTS(asound snd_card_next "" HAVE_ALSA_LIB)
    IF(NOT HAVE_ALSA_INC OR NOT HAVE_ALSA_LIB)

        SET(WITH_ALSA OFF
            CACHE BOOL "alsa files could not be found on this system. Forcing this option to OFF"
            FORCE)
        MESSAGE("WARNING: alsa files could not be found on this system. Compiling without alsa support")

    ENDIF(NOT HAVE_ALSA_INC OR NOT HAVE_ALSA_LIB)
ENDIF(WITH_ALSA)


IF (WITH_ALSA)
    message(STATUS "   Compiling with alsa support")
    add_definitions( -DHAVE_ALSA )
ELSE(WITH_ALSA)
    message(STATUS "   Compiling without alsa support")
ENDIF(WITH_ALSA)

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

###############################################################################




option(WITH_FFMPEG "Compile with Internet Radio Stations support (libavformat/libffmpeg, libmms)" ON)

IF(WITH_FFMPEG)
    SET (FFMPEG_LDFLAGS   "")
    SET (FFMPEG_CFLAGS    "")
    SET (FFMPEG_LIBRARIES "")

    pkg_check_modules(LIBAVFORMAT REQUIRED libavformat)
    IF(LIBAVFORMAT_FOUND)
        SET(HAVE_AVFORMAT_INC ON)
        SET(HAVE_AVFORMAT_LIB ON)
        IF(LIBAVFORMAT_CFLAGS)
            IF(FFMPEG_CFLAGS)
                SET(FFMPEG_CFLAGS    "${LIBAVFORMAT_CFLAGS}       ${FFMPEG_CFLAGS}")
            ELSE(FFMPEG_CFLAGS)
                SET(FFMPEG_CFLAGS    "${LIBAVFORMAT_CFLAGS}")
            ENDIF(FFMPEG_CFLAGS)
        ENDIF(LIBAVFORMAT_CFLAGS)
        IF(LIBAVFORMAT_LDFLAGS)
            IF(FFMPEG_LDFLAGS)
                SET(FFMPEG_LDFLAGS   "${LIBAVFORMAT_LDFLAGS}      ${FFMPEG_LDFLAGS}")
            ELSE(FFMPEG_LDFLAGS)
                SET(FFMPEG_LDFLAGS   "${LIBAVFORMAT_LDFLAGS}")
            ENDIF(FFMPEG_LDFLAGS)
        ENDIF(LIBAVFORMAT_LDFLAGS)
        IF(LIBAVFORMAT_LIBRARIES)
            IF(FFMPEG_LIBRARIES)
                SET(FFMPEG_LIBRARIES "${LIBAVFORMAT_LIBRARIES}    ${FFMPEG_LIBRARIES}")
            ELSE(FFMPEG_LIBRARIES)
                SET(FFMPEG_LIBRARIES "${LIBAVFORMAT_LIBRARIES}")
            ENDIF(FFMPEG_LIBRARIES)
        ENDIF(LIBAVFORMAT_LIBRARIES)
    ELSE(LIBAVFORMAT_FOUND)
        CHECK_INCLUDE_FILE_CXX(ffmpeg/avformat.h            HAVE_AVFORMAT_OLD_INC)
        CHECK_INCLUDE_FILE_CXX(libavformat/avformat.h       HAVE_AVFORMAT_INC)
        CHECK_LIBRARY_EXISTS(avformat av_open_input_file "" HAVE_AVFORMAT_LIB)
        IF (HAVE_AVFORMAT_OLD_INC OR HAVE_AVFORMAT_INC)
            IF (HAVE_AVFORMAT_LIB)
                SET(LIBAVFORMAT_FOUND ON)
                IF(FFMPEG_LIBRARIES)
                    SET(FFMPEG_LIBRARIES "-lavutil -lavformat ${FFMPEG_LIBRARIES}")
                ELSE(FFMPEG_LIBRARIES)
                    SET(FFMPEG_LIBRARIES "-lavutil -lavformat")
                ENDIF(FFMPEG_LIBRARIES)
                IF(FFMPEG_LDFLAGS)
                    SET(FFMPEG_LDFLAGS   "-lavutil -lavformat ${FFMPEG_LDFLAGS}")
                ELSE(FFMPEG_LDFLAGS)
                    SET(FFMPEG_LDFLAGS   "-lavutil -lavformat")
                ENDIF(FFMPEG_LDFLAGS)
            ENDIF(HAVE_AVFORMAT_LIB)
        ENDIF(HAVE_AVFORMAT_OLD_INC OR HAVE_AVFORMAT_INC)
    ENDIF(LIBAVFORMAT_FOUND)

    pkg_check_modules(LIBMMS      REQUIRED libmms)
    IF(LIBMMS_FOUND)
        SET(HAVE_MMSX_INC ON)
        SET(HAVE_MMSX_LIB ON)
        IF(LIBMMS_CFLAGS)
            IF(FFMPEG_CFLAGS)
                SET(FFMPEG_CFLAGS    "${LIBMMS_CFLAGS}       ${FFMPEG_CFLAGS}")
            ELSE(FFMPEG_CFLAGS)
                SET(FFMPEG_CFLAGS    "${LIBMMS_CFLAGS}")
            ENDIF(FFMPEG_CFLAGS)
        ENDIF(LIBMMS_CFLAGS)
        IF(LIBMMS_LDFLAGS)
            IF(FFMPEG_LDFLAGS)
                SET(FFMPEG_LDFLAGS   "${LIBMMS_LDFLAGS}      ${FFMPEG_LDFLAGS}")
            ELSE(FFMPEG_LDFLAGS)
                SET(FFMPEG_LDFLAGS   "${LIBMMS_LDFLAGS}")
            ENDIF(FFMPEG_LDFLAGS)
        ENDIF(LIBMMS_LDFLAGS)
        IF(LIBMMS_LIBRARIES)
            IF(FFMPEG_LIBRARIES)
                SET(FFMPEG_LIBRARIES "${LIBMMS_LIBRARIES}    ${FFMPEG_LIBRARIES}")
            ELSE(FFMPEG_LIBRARIES)
                SET(FFMPEG_LIBRARIES "${LIBMMS_LIBRARIES}")
            ENDIF(FFMPEG_LIBRARIES)
        ENDIF(LIBMMS_LIBRARIES)
    ELSE(LIBMMS_FOUND)
        CHECK_INCLUDE_FILE("libmms/mmsx.h"       HAVE_MMSX_INC)
        CHECK_LIBRARY_EXISTS(mms mmsx_connect "" HAVE_MMSX_LIB)
        IF (HAVE_MMSX_INC AND HAVE_MMSX_LIB)
            SET(LIBMMS_FOUND ON)
                IF(FFMPEG_LIBRARIES)
                    SET(FFMPEG_LIBRARIES "-lmms ${FFMPEG_LIBRARIES}")
                ELSE(FFMPEG_LIBRARIES)
                    SET(FFMPEG_LIBRARIES "-lmms")
                ENDIF(FFMPEG_LIBRARIES)
                IF(FFMPEG_LDFLAGS)
                    SET(FFMPEG_LDFLAGS   "-lmms ${FFMPEG_LDFLAGS}")
                ELSE(FFMPEG_LDFLAGS)
                    SET(FFMPEG_LDFLAGS   "-lmms")
                ENDIF(FFMPEG_LDFLAGS)
        ENDIF(HAVE_MMSX_INC AND HAVE_MMSX_LIB)
    ENDIF(LIBMMS_FOUND)

    IF ( NOT LIBMMS_FOUND AND NOT LIBAVFORMAT_FOUND )
        SET(WITH_FFMPEG OFF
            CACHE BOOL "libffmpeg and/or libmms files could not be found on this system. Forcing this option to OFF"
            FORCE)
        MESSAGE("WARNING: libffmpeg and/or libmms files could not be found on this system. Compiling without Internet Radio Stations support")
    ENDIF( NOT LIBMMS_FOUND AND NOT LIBAVFORMAT_FOUND )

ENDIF(WITH_FFMPEG)


IF (WITH_FFMPEG)
    message(STATUS "   Compiling with Internet Radio Stations (libavformat/libffmpeg, libmms) support")

    IF(HAVE_AVFORMAT_OLD_INC)
        add_definitions( -DHAVE_FFMPEG_OLD )
    ELSE(HAVE_AVFORMAT_OLD_INC)
        add_definitions( -DHAVE_FFMPEG )
    ENDIF(HAVE_AVFORMAT_OLD_INC)

    add_definitions( -DHAVE_MMS )
ELSE(WITH_FFMPEG)
    message(STATUS "   Compiling without Internet Radio Stations (libffmpeg, libmms) support")
ENDIF(WITH_FFMPEG)

###############################################################################



ADD_SUBDIRECTORY(radio)
ADD_SUBDIRECTORY(v4lradio)
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
