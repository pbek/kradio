project(kradio4_plugin_recording)
add_definitions( -DPROJECT_NAME="\\"${PROJECT_NAME}\\"" )

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

include(CheckIncludeFile)

ADD_SUBDIRECTORY(icons)
ADD_SUBDIRECTORY(po)

option(WITH_OGG_VORBIS "Compile with ogg/vorbis support" ON)
option(WITH_LAME       "Compile with mp3/lame   support" ON)

###############################################################################

IF(WITH_OGG_VORBIS)
    CHECK_INCLUDE_FILE(vorbis/vorbisenc.h HAVE_VORBIS_INC)
    CHECK_INCLUDE_FILE(ogg/ogg.h          HAVE_OGG_INC)
    CHECK_LIBRARY_EXISTS(vorbisenc vorbis_encode_init  "" HAVE_VORBIS_LIB)
    CHECK_LIBRARY_EXISTS(ogg       ogg_stream_packetin "" HAVE_OGG_LIB)
    IF(NOT HAVE_VORBIS_INC OR NOT HAVE_OGG_INC OR NOT HAVE_VORBIS_LIB OR NOT HAVE_OGG_LIB)

        SET(WITH_OGG_VORBIS OFF
            CACHE BOOL "ogg/vorbis files could not be found on this system. Forcing this option to OFF"
            FORCE)
        MESSAGE("WARNING: ogg/vorbis files could not be found on this system. Compiling without ogg/vorbis support")

    ENDIF(NOT HAVE_VORBIS_INC OR NOT HAVE_OGG_INC OR NOT HAVE_VORBIS_LIB OR NOT HAVE_OGG_LIB)
ENDIF(WITH_OGG_VORBIS)


IF (WITH_OGG_VORBIS)
    message(STATUS "   Compiling with ogg/vorbis support")
    add_definitions( -DHAVE_OGG )
    set(LIB_OGG "-logg -lvorbisenc")
ELSE(WITH_OGG_VORBIS)
    message(STATUS "   Compiling without ogg/vorbis support")
    set(LIB_OGG "")
ENDIF(WITH_OGG_VORBIS)

MACRO_LOG_FEATURE(WITH_OGG_VORBIS "ogg/vorbis support"  "Support for audio compression format ogg/vorbis" "http://www.vorbis.com") 

###############################################################################


IF(WITH_LAME)
    CHECK_INCLUDE_FILE(lame/lame.h HAVE_LAME_INC)
    CHECK_LIBRARY_EXISTS(mp3lame lame_init "" HAVE_LAME_LIB)
    IF(NOT HAVE_LAME_INC OR NOT HAVE_LAME_LIB)

        SET(WITH_LAME OFF
            CACHE BOOL "mp3/lame files could not be found on this system. Forcing this option to OFF"
            FORCE)
        MESSAGE("WARNING: mp3/lame files could not be found on this system. Compiling without mp3/lame support")

    ENDIF(NOT HAVE_LAME_INC OR NOT HAVE_LAME_LIB)
ENDIF(WITH_LAME)


IF (WITH_LAME)
    message(STATUS "   Compiling with mp3/lame support")
    add_definitions( -DHAVE_LAME )
    set(LIB_LAME "-lmp3lame")
ELSE(WITH_LAME)
    message(STATUS "   Compiling without mp3/lame support")
    set(LIB_LAME "")
ENDIF(WITH_LAME)

MACRO_LOG_FEATURE(WITH_LAME "mp3/lame support"  "Support for audio compression format .mp3 via lame" "http://lame.sourceforge.net") 

###############################################################################

SET(WITH_LIBSNDFILE ON)

CHECK_INCLUDE_FILE(sndfile.h HAVE_SNDFILE_INC)
CHECK_LIBRARY_EXISTS(sndfile sf_open "" HAVE_SNDFILE_LIB)
IF(NOT HAVE_SNDFILE_INC OR NOT HAVE_SNDFILE_LIB)

    SET(WITH_LIBSNDFILE OFF)
    MESSAGE(SEND_ERROR "libsndfile files could not be found on this system.")

ENDIF(NOT HAVE_SNDFILE_INC OR NOT HAVE_SNDFILE_LIB)

MACRO_LOG_FEATURE(WITH_LIBSNDFILE "libsndfile generic sound file support"  "Support for some standard audio formats (e.g. .wav)" "http://www.mega-nerd.com/libsndfile" TRUE)

###############################################################################


FILE(GLOB kradio_plugin_recording_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

FILE(GLOB kradio_plugin_recording_include
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

FILE(GLOB kradio_plugin_recording_UIs
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)

kde4_add_ui_files(kradio_plugin_recording_sources
    ${kradio_plugin_recording_UIs}
)

KDE4_INSTALL_ICONS( ${ICON_INSTALL_DIR}  )


string(REGEX REPLACE "-Wl,--no-undefined" "" CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS})


kde4_add_plugin(kradio4_plugin_recording
    ${kradio_plugin_recording_sources}
)

SET_TARGET_PROPERTIES(kradio4_plugin_recording
    PROPERTIES
    VERSION       ${KRADIO_VERSION}
    SOVERSION     4

)

TARGET_LINK_LIBRARIES(kradio4_plugin_recording
  ${KDE4_KDEUI_LIBS}
  ${QT_QT3SUPPORT_LIBRARY}
  ${KDE4_KDE3SUPPORT_LIBRARY}
  ${LIB_OGG}
  ${LIB_LAME}
  -lsndfile
)

INSTALL(TARGETS kradio4_plugin_recording ${INSTALL_TARGETS_DEFAULT_ARGS})

