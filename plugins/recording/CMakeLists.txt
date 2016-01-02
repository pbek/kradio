project(kradio4_plugin_recording)
add_definitions( -DPROJECT_NAME="${PROJECT_NAME}" )

include(CheckIncludeFile)

ADD_SUBDIRECTORY(icons)
ADD_SUBDIRECTORY(po)

option(WITH_OGG_VORBIS "Compile with ogg/vorbis support" ON)
option(WITH_LAME       "Compile with mp3/lame   support" ON)

###############################################################################

IF(WITH_OGG_VORBIS)
    find_package(OggVorbis)
    IF(NOT OGGVORBIS_FOUND OR NOT HAVE_LIBVORBISENC2)

        SET(WITH_OGG_VORBIS OFF
            CACHE BOOL "ogg/vorbis files could not be found on this system. Forcing this option to OFF"
            FORCE)
        MESSAGE("WARNING: ogg/vorbis files could not be found on this system. Compiling without ogg/vorbis support")

    ENDIF(NOT OGGVORBIS_FOUND OR NOT HAVE_LIBVORBISENC2)
ENDIF(WITH_OGG_VORBIS)


IF (WITH_OGG_VORBIS)
    message(STATUS "   Compiling with ogg/vorbis support")
    add_definitions( -DHAVE_OGG )
    include_directories(${OGGVORBIS_INCLUDE_DIR} ${VORBIS_INCLUDE_DIR})
    set(LIB_OGG ${OGGVORBIS_LIBRARIES})
ELSE(WITH_OGG_VORBIS)
    message(STATUS "   Compiling without ogg/vorbis support")
    set(LIB_OGG "")
ENDIF(WITH_OGG_VORBIS)

MACRO_LOG_FEATURE(WITH_OGG_VORBIS "ogg/vorbis support"  "Support for audio compression format ogg/vorbis" "http://www.vorbis.com") 

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

MACRO_LOG_FEATURE(WITH_LAME "mp3/lame support"  "Support for audio compression format .mp3 via lame" "http://lame.sourceforge.net") 

###############################################################################

SET(WITH_LIBSNDFILE ON)

find_package(Sndfile REQUIRED)
SET(WITH_LIBSNDFILE ${Sndfile_FOUND})

include_directories(${Sndfile_INCLUDE_DIRS})

MACRO_LOG_FEATURE(WITH_LIBSNDFILE "libsndfile generic sound file support"  "Support for some standard audio formats (e.g. .wav)" "http://www.mega-nerd.com/libsndfile" TRUE)

###############################################################################


FILE(GLOB kradio_plugin_recording_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
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

TARGET_LINK_LIBRARIES(kradio4_plugin_recording
  ${KDE4_KDEUI_LIBS}
  ${LIB_OGG}
  ${LIB_LAME}
  ${Sndfile_LIBRARIES}
)

INSTALL(TARGETS kradio4_plugin_recording ${INSTALL_TARGETS_DEFAULT_ARGS})

