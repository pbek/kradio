project(kradio4_plugin_internetradio)
add_definitions( -DPROJECT_NAME="\\"${PROJECT_NAME}\\"" )

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)



#add_definitions( -DINET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD )


#ADD_SUBDIRECTORY(icons)
ADD_SUBDIRECTORY(po)

LINK_DIRECTORIES(
    ${FFMPEG_LIBRARY_DIRS}
)

INCLUDE_DIRECTORIES(
    ${FFMPEG_INCLUDE_DIRS}
)

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
    VERSION       ${KRADIO_VERSION}
    SOVERSION     4
)

TARGET_LINK_LIBRARIES(kradio4_plugin_internetradio
  ${KDE4_KDEUI_LIBS}
  ${QT_QT3SUPPORT_LIBRARY}
  ${KDE4_KDE3SUPPORT_LIBRARY}
  ${FFMPEG_LIBRARIES}
)

INSTALL(TARGETS kradio4_plugin_internetradio ${INSTALL_TARGETS_DEFAULT_ARGS})
