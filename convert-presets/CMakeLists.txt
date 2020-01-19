# Locate extra-cmake-modules version 0.0.11 and make it a required package
find_package(ECM 5.10.0 REQUIRED NO_MODULE)

########################################################################
#       KDE/QT Libs                                                    #
########################################################################

# Locate Qt5, make it required package and use QtWidgets and QtCore modules
find_package(Qt5 REQUIRED COMPONENTS Core )

# Locate KF5, make it required package and use KF5KDELibs4Support and KF5CoreAddons modules
find_package(KF5 REQUIRED COMPONENTS  CoreAddons I18n)


########################################################################


add_executable(kradio5-convert-presets
    convert-presets.cpp
)

TARGET_LINK_LIBRARIES(kradio5-convert-presets
    Qt5::Core
    KF5::CoreAddons
    KF5::I18n
)

INSTALL(TARGETS kradio5-convert-presets ${INSTALL_TARGETS_DEFAULT_ARGS})

