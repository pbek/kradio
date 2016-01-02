project(kradio4_plugin_dbus)
add_definitions( -DPROJECT_NAME="${PROJECT_NAME}" )

ADD_SUBDIRECTORY(po)

FILE(GLOB kradio_plugin_dbus_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

FILE(GLOB kradio_plugin_dbus_UIs
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)

kde4_add_ui_files(kradio_plugin_dbus_sources
    ${kradio_plugin_dbus_UIs}
)

QT4_ADD_DBUS_ADAPTOR(kradio_plugin_dbus_sources net.sourceforge.kradio.xml
                     dbussupport.h DBusSupport)


string(REGEX REPLACE "-Wl,--no-undefined" "" CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS})


kde4_add_plugin(kradio4_plugin_dbus
    ${kradio_plugin_dbus_sources}
)

TARGET_LINK_LIBRARIES(kradio4_plugin_dbus
  ${KDE4_KDEUI_LIBS}
)

INSTALL(TARGETS kradio4_plugin_dbus ${INSTALL_TARGETS_DEFAULT_ARGS})


