
set(kwin_xml  ${CMAKE_CURRENT_SOURCE_DIR}/../org.kde.KWin.xml)

########### next target ###############
set(kwin_update_default_rules_SRCS update_default_rules.cpp)


add_executable( kwin_update_default_rules ${kwin_update_default_rules_SRCS})

target_link_libraries( kwin_update_default_rules  )

install(TARGETS kwin_update_default_rules DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/  )

########### next target ###############
set( kwin_update_tabbox_settings_SRCS update_tabbox_settings.cpp )

add_executable( kwin_update_tabbox_settings ${kwin_update_tabbox_settings_SRCS} )

target_link_libraries( kwin_update_tabbox_settings  )

install( TARGETS kwin_update_tabbox_settings DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/ )
########### next target ###############
set( kwin_update_tabbox_qml_settings_SRCS update_tabbox_qml_settings.cpp )

add_executable( kwin_update_tabbox_qml_settings ${kwin_update_tabbox_qml_settings_SRCS} )

target_link_libraries( kwin_update_tabbox_qml_settings  )

install( TARGETS kwin_update_tabbox_qml_settings DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/ )

########### next target ###############
set( kwin_update_settings_49_SRCS update_kwin_49.cpp update_kwin_49_main.cpp )

add_executable( kwin_update_settings_49 ${kwin_update_settings_49_SRCS} )

target_link_libraries( kwin_update_settings_49  )

install( TARGETS kwin_update_settings_49 DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/ )

########### next target ###############
set( kwin_update_settings_410_SRCS update_kwin_410.cpp )

add_executable( kwin_update_settings_410 ${kwin_update_settings_410_SRCS} )

target_link_libraries( kwin_update_settings_410  )

install( TARGETS kwin_update_settings_410 DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/ )

########### next target ###############
set( kwin_update_settings_411_SRCS update_kwin_411.cpp )

add_executable( kwin_update_settings_411 ${kwin_update_settings_411_SRCS} )

target_link_libraries( kwin_update_settings_411  )

install( TARGETS kwin_update_settings_411 DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin/ )

########### install files ###############

install( FILES fsp_workarounds_1.kwinrules  DESTINATION  ${DATA_INSTALL_DIR}/kwin/default_rules )
install( FILES pop.wav  DESTINATION  ${SOUND_INSTALL_DIR} )
install( FILES kwin_fsp_workarounds_1.upd kwin_update_tabbox_settings.upd kwin_remove_effects.upd kwin_update_tabbox_qml_settings.upd kwin_remove_delay_focus.upd kwin_update_49.upd kwin_update_410.upd kwin_translate_activity_rule.upd kwin_translate_confrequest_rule.upd
               kwin_update_411.upd
         DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
install( PROGRAMS kwin_remove_delay_focus.sh kwin_translate_activity_rule.sh kwin_translate_confrequest_rule.sh DESTINATION  ${KCONF_UPDATE_INSTALL_DIR} )
install( FILES stripTitle.js  DESTINATION  ${DATA_INSTALL_DIR}/kwin )

