project(kradio4_plugin_v4lradio)
add_definitions( -DPROJECT_NAME="${PROJECT_NAME}" )

ADD_SUBDIRECTORY(icons)
ADD_SUBDIRECTORY(po)



FILE(GLOB kradio_plugin_v4lradio_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

FILE(GLOB kradio_plugin_v4lradio_UIs
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)

kde4_add_ui_files(kradio_plugin_v4lradio_sources
    ${kradio_plugin_v4lradio_UIs}
)

KDE4_INSTALL_ICONS( ${ICON_INSTALL_DIR}  )


string(REGEX REPLACE "-Wl,--no-undefined" "" CMAKE_MODULE_LINKER_FLAGS ${CMAKE_MODULE_LINKER_FLAGS})


kde4_add_plugin(kradio4_plugin_v4lradio
    ${kradio_plugin_v4lradio_sources}
)

TARGET_LINK_LIBRARIES(kradio4_plugin_v4lradio
  ${KDE4_KDEUI_LIBS}
)

INSTALL(TARGETS kradio4_plugin_v4lradio ${INSTALL_TARGETS_DEFAULT_ARGS})

###############################################################################

option(WITH_V4L2 "Compile with V4L2 support" ON)

IF (WITH_V4L2)
    message(STATUS "   Compiling with V4L2 support")
    add_definitions( -DHAVE_V4L2 )
ELSE(WITH_V4L2)
    message(STATUS "   Compiling without V4L2 support")
ENDIF(WITH_V4L2)

MACRO_LOG_FEATURE(WITH_V4L2 "V4L2 Support"  "Support for V4L2 support. Recommended and necessary for recent kernels > 2.6.30" "http://www.linuxtv.org/wiki/index.php/Main_Page")

###############################################################################


option(WITH_RDS_DEBUG_EMULATE "Do some RDS emulation for RDS debugging without an RDS-enabled card" OFF)

IF (WITH_RDS_DEBUG_EMULATE)
    message(STATUS "   Compiling with RDS-Debug-Emulation")
    add_definitions( -DRDS_DEBUG_EMULATE )
ELSE(WITH_RDS_DEBUG_EMULATE)
    message(STATUS "   Compiling without RDS-Debug-Emulation")
ENDIF(WITH_RDS_DEBUG_EMULATE)

#MACRO_LOG_FEATURE(WITH_RDS_DEBUG_EMULATE "RDS Debug Emulation"  "Emulate some dummy RDS message for testing RDS functionality without having an RDS renabled radio card" "http://www.linuxtv.org/wiki/index.php/Radio_Data_System_%28RDS%29")

###############################################################################

