set(notifications-to-orgkdenotifications_SRCS notifications-to-orgkdenotifications.cpp)
add_executable(notifications-to-orgkdenotifications ${notifications-to-orgkdenotifications_SRCS})
target_link_libraries(notifications-to-orgkdenotifications )
install(TARGETS notifications-to-orgkdenotifications DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/)

install(FILES notifications-to-orgkdenotifications.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR})

set(plasma_add_shortcut_to_menu_SRCS plasma-add-shortcut-to-menu.cpp)
add_executable(plasma-add-shortcut-to-menu ${plasma_add_shortcut_to_menu_SRCS})
target_link_libraries(plasma-add-shortcut-to-menu )
install(TARGETS plasma-add-shortcut-to-menu DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/)

install(FILES plasma-add-shortcut-to-menu.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR})

set(plasma_to_plasma_desktop_SRCS plasma-to-plasma-desktop.cpp)
add_executable(plasma-to-plasma-desktop ${plasma_to_plasma_desktop_SRCS})
target_link_libraries(plasma-to-plasma-desktop KIOCore)
install(TARGETS plasma-to-plasma-desktop DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/)

install(FILES plasmarc-to-plasmadesktoprc.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR})
install(FILES plasma-to-plasmadesktop-shortcuts.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR})

install(FILES systray-to-notifications-widget.js
              addShowActivitiesManagerPlasmoid.js
              set-hidden-single-vd-pager.js
        DESTINATION ${DATA_INSTALL_DIR}/plasma-desktop/updates/ )
