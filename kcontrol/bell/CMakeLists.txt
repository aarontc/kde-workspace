


########### next target ###############

set(kcm_bell_PART_SRCS bell.cpp )


add_library(kcm_bell MODULE ${kcm_bell_PART_SRCS})


target_link_libraries(kcm_bell
    Qt5::X11Extras
    KF5::KCMUtils
    KF5::KI18n
    KF5::KNotifications
    KF5::KWindowSystem
    KF5::KDE4Support
    ${X11_X11_LIB})

install(TARGETS kcm_bell  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES bell.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

ecm_install_icons( ${ICON_INSTALL_DIR}  )
