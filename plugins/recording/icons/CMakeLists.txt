file(GLOB kradio5_recording_icons [0-9]*.png [0-9]*.svg)

ecm_install_icons(ICONS ${kradio5_recording_icons} DESTINATION ${ICON_INSTALL_DIR}  )
