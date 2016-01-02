project(kradio4_plugin_alsa_sound)

add_definitions( -DPROJECT_NAME="${PROJECT_NAME}" )

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

kde4_add_ui_files(kradio_plugin_alsa_sound_sources
    ${kradio_plugin_alsa_sound_UIs}
)

KDE4_INSTALL_ICONS( ${ICON_INSTALL_DIR}  )


string(REGEX REPLACE "-Wl,--no-undefined" "" CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS})


kde4_add_plugin(kradio4_plugin_alsa_sound
    ${kradio_plugin_alsa_sound_sources}
)

TARGET_LINK_LIBRARIES(kradio4_plugin_alsa_sound
  ${KDE4_KDEUI_LIBS}
  ${ALSA_LIBRARIES}
)

INSTALL(TARGETS kradio4_plugin_alsa_sound ${INSTALL_TARGETS_DEFAULT_ARGS})

