########### next target ###############

set(kcm_kwinoptions_PART_SRCS windows.cpp mouse.cpp main.cpp )
kde4_add_ui_files(kcm_kwinoptions_PART_SRCS actions.ui advanced.ui focus.ui mouse.ui moving.ui)
add_library(kcm_kwinoptions MODULE ${kcm_kwinoptions_PART_SRCS})
target_link_libraries(kcm_kwinoptions Qt5::DBus KF5::KCompletion KF5::KI18n KF5::KCMUtils KF5::KDE4Support)
install(TARGETS kcm_kwinoptions  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kwinoptions.desktop kwinactions.desktop kwinadvanced.desktop
    kwinfocus.desktop kwinmoving.desktop
    DESTINATION ${SERVICES_INSTALL_DIR})

ecm_install_icons( ${ICON_INSTALL_DIR} )
