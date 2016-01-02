project(kradio4_plugin_lirc)
add_definitions( -DPROJECT_NAME="${PROJECT_NAME}" )

ADD_SUBDIRECTORY(po)

INCLUDE_DIRECTORIES(
    ${LircClient_INCLUDE_DIRS}
)

FILE(GLOB kradio_plugin_lirc_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

FILE(GLOB kradio_plugin_lirc_UIs
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)

kde4_add_ui_files(kradio_plugin_lirc_sources
    ${kradio_plugin_lirc_UIs}
)

KDE4_INSTALL_ICONS( ${ICON_INSTALL_DIR}  )


string(REGEX REPLACE "-Wl,--no-undefined" "" CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS})


kde4_add_plugin(kradio4_plugin_lirc
    ${kradio_plugin_lirc_sources}
)

TARGET_LINK_LIBRARIES(kradio4_plugin_lirc
  ${KDE4_KDEUI_LIBS}
  ${LircClient_LIBRARIES}
)

INSTALL(TARGETS kradio4_plugin_lirc ${INSTALL_TARGETS_DEFAULT_ARGS})

INSTALL(FILES default-dot-lircrc DESTINATION ${DATA_INSTALL_DIR}/kradio4 )

