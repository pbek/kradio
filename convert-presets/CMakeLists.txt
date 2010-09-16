
ADD_DEFINITIONS(
    -DQT3_SUPPORT
    -DKDE3_SUPPORT
    -DQT3_SUPPORT_WARNINGS
)


kde4_add_executable(kradio4-convert-presets
    convert-presets.cpp
)

TARGET_LINK_LIBRARIES(kradio4-convert-presets
  ${KDE4_KDEUI_LIBS}
  ${KDE4_KPARTS_LIBS}
  ${QT_QT3SUPPORT_LIBRARY}
  ${KDE4_KDE3SUPPORT_LIBRARY}
)

INSTALL(TARGETS kradio4-convert-presets ${INSTALL_TARGETS_DEFAULT_ARGS})

