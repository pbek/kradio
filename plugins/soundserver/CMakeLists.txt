project(kradio4_plugin_soundserver)
add_definitions( -DPROJECT_NAME="\\"${PROJECT_NAME}\\"" )

ADD_SUBDIRECTORY(po)

FILE(GLOB kradio_plugin_soundserver_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

FILE(GLOB kradio_plugin_soundserver_include
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

FILE(GLOB kradio_plugin_soundserver_UIs
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)


KDE4_ADD_UI_FILES(kradio_plugin_soundserver_sources
    ${kradio_plugin_soundserver_UIs}
)

KDE4_INSTALL_ICONS( ${ICON_INSTALL_DIR}  )



KDE4_ADD_PLUGIN(kradio4_plugin_soundserver
    ${kradio_plugin_soundserver_sources}
)




string(REGEX REPLACE "-Wl,--no-undefined" "" CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS})

TARGET_LINK_LIBRARIES(kradio4_plugin_soundserver 
  ${KDE4_KDEUI_LIBS}
)

INSTALL(TARGETS kradio4_plugin_soundserver ${INSTALL_TARGETS_DEFAULT_ARGS})

