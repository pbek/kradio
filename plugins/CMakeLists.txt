project(kradio4plugins)

# search packages used by KDE
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
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




option(WITH_FFMPEG "Compile with Internet Radio Stations support (libffmpeg, libmms)" ON)

IF(WITH_FFMPEG)
    CHECK_INCLUDE_FILE_CXX(libavformat/avformat.h HAVE_AVFORMAT_INC)
    CHECK_LIBRARY_EXISTS(avformat av_open_input_file "" HAVE_AVFORMAT_LIB)
    CHECK_INCLUDE_FILE("libmms/mmsx.h" HAVE_MMSX_INC)
    CHECK_LIBRARY_EXISTS(mms mmsx_connect "" HAVE_MMSX_LIB)
    IF(NOT HAVE_AVFORMAT_INC OR NOT HAVE_AVFORMAT_LIB OR NOT HAVE_MMSX_INC OR NOT HAVE_MMSX_LIB)

        SET(WITH_FFMPEG OFF
            CACHE BOOL "libffmpeg and/or libmms files could not be found on this system. Forcing this option to OFF"
            FORCE)
        MESSAGE("WARNING: libffmpeg and/or libmms files could not be found on this system. Compiling without Internet Radio Stations support")

    ENDIF(NOT HAVE_AVFORMAT_INC OR NOT HAVE_AVFORMAT_LIB OR NOT HAVE_MMSX_INC OR NOT HAVE_MMSX_LIB)
ENDIF(WITH_FFMPEG)


IF (WITH_FFMPEG)
    message(STATUS "   Compiling with Internet Radio Stations (libffmpeg, libmms) support")
    add_definitions( -DHAVE_FFMPEG )
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
