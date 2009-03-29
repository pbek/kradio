project(kradio4_plugin_internetradio)
add_definitions( -DPROJECT_NAME="\\"${PROJECT_NAME}\\"" )

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)



#ADD_SUBDIRECTORY(icons)
ADD_SUBDIRECTORY(po)

FILE(GLOB kradio_plugin_internetradio_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

FILE(GLOB kradio_plugin_internetradio_include
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

FILE(GLOB kradio_plugin_internetradio_UIs
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)

kde4_add_ui_files(kradio_plugin_internetradio_sources
    ${kradio_plugin_internetradio_UIs}
)

KDE4_INSTALL_ICONS( ${ICON_INSTALL_DIR}  )


string(REGEX REPLACE "-Wl,--no-undefined" "" CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS})


kde4_add_plugin(kradio4_plugin_internetradio
    ${kradio_plugin_internetradio_sources}
)

SET_TARGET_PROPERTIES(kradio4_plugin_internetradio
    PROPERTIES
    VERSION       4.0.0
    SOVERSION     4

)

TARGET_LINK_LIBRARIES(kradio4_plugin_internetradio
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KPARTS_LIBS}
  ${QT_QT3SUPPORT_LIBRARY}
  ${KDE4_KDE3SUPPORT_LIBRARY}
  -lavutil
  -lavformat
  -lmms
)

INSTALL(TARGETS kradio4_plugin_internetradio ${INSTALL_TARGETS_DEFAULT_ARGS})