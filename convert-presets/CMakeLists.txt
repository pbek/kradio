
kde4_add_executable(kradio4-convert-presets
    convert-presets.cpp
)

TARGET_LINK_LIBRARIES(kradio4-convert-presets
  ${KDE4_KDECORE_LIBS}
)

INSTALL(TARGETS kradio4-convert-presets ${INSTALL_TARGETS_DEFAULT_ARGS})

