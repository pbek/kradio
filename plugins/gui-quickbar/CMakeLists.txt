project(kradio4_plugin_gui_quickbar)
add_definitions( -DPROJECT_NAME="\\"${PROJECT_NAME}\\"" )

ADD_SUBDIRECTORY(po)



FILE(GLOB kradio_plugin_gui_quickbar_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

FILE(GLOB kradio_plugin_gui_quickbar_include
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

FILE(GLOB kradio_plugin_gui_quickbar_UIs
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)

kde4_add_ui_files(kradio_plugin_gui_quickbar_sources
    ${kradio_plugin_gui_quickbar_UIs}
)



string(REGEX REPLACE "-Wl,--no-undefined" "" CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS})


kde4_add_plugin(kradio4_plugin_gui_quickbar
    ${kradio_plugin_gui_quickbar_sources}
)

TARGET_LINK_LIBRARIES(kradio4_plugin_gui_quickbar
  ${KDE4_KDEUI_LIBS}
)

INSTALL(TARGETS kradio4_plugin_gui_quickbar ${INSTALL_TARGETS_DEFAULT_ARGS})

