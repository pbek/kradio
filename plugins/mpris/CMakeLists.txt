project(kradio4_plugin_mpris)
add_definitions( -DPROJECT_NAME="\\"${PROJECT_NAME}\\"" )

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)



ADD_SUBDIRECTORY(po)

FILE(GLOB kradio_plugin_mpris_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

FILE(GLOB kradio_plugin_mpris_include
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

FILE(GLOB kradio_plugin_mpris_UIs
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)

kde4_add_ui_files(kradio_plugin_mpris_sources
    ${kradio_plugin_mpris_UIs}
)

KDE4_INSTALL_ICONS( ${ICON_INSTALL_DIR}  )


string(REGEX REPLACE "-Wl,--no-undefined" "" CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS})


kde4_add_plugin(kradio4_plugin_mpris
    ${kradio_plugin_mpris_sources}
)

TARGET_LINK_LIBRARIES(kradio4_plugin_mpris
  ${KDE4_KDEUI_LIBS}
)

INSTALL(TARGETS kradio4_plugin_mpris ${INSTALL_TARGETS_DEFAULT_ARGS})


