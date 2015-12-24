project(kradio4_plugin_timecontrol)
add_definitions( -DPROJECT_NAME="${PROJECT_NAME}" )

ADD_SUBDIRECTORY(icons)
ADD_SUBDIRECTORY(po)



FILE(GLOB kradio_plugin_timecontrol_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

FILE(GLOB kradio_plugin_timecontrol_include
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

FILE(GLOB kradio_plugin_timecontrol_UIs
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)

kde4_add_ui_files(kradio_plugin_timecontrol_sources
    ${kradio_plugin_timecontrol_UIs}
)

KDE4_INSTALL_ICONS( ${ICON_INSTALL_DIR}  )


string(REGEX REPLACE "-Wl,--no-undefined" "" CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS})


kde4_add_plugin(kradio4_plugin_timecontrol
    ${kradio_plugin_timecontrol_sources}
)

TARGET_LINK_LIBRARIES(kradio4_plugin_timecontrol
  ${KDE4_KDEUI_LIBS}
)

INSTALL(TARGETS kradio4_plugin_timecontrol ${INSTALL_TARGETS_DEFAULT_ARGS})

