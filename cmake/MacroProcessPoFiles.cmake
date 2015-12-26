macro(process_po_files)
    if(GETTEXT_MSGFMT_EXECUTABLE)
        set(catalogname ${PROJECT_NAME})
        file(GLOB _poFiles ${CMAKE_CURRENT_SOURCE_DIR}/*.po)
        set(_gmoFiles)

        foreach(_poFile ${_poFiles})
            get_filename_component(_poFileName "${_poFile}" NAME)
            get_filename_component(_lang "${_poFile}" NAME_WE)
            set(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)
            add_custom_command(
                OUTPUT ${_gmoFile}
                COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_poFile}
                DEPENDS ${_poFile}
            )
            install(
                FILES ${_gmoFile}
                DESTINATION ${LOCALE_INSTALL_DIR}/${_lang}/LC_MESSAGES/
                RENAME ${catalogname}.mo
            )
            list(APPEND _gmoFiles ${_gmoFile})
        endforeach()

        add_custom_target(translations ALL DEPENDS ${_gmoFiles})
    endif()
endmacro()
