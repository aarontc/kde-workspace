
set( kcm_powerdevil_activities_SRCS
     activitypage.cpp
     activitywidget.cpp
     ../common/ErrorOverlay.cpp
)

kde4_add_ui_files(kcm_powerdevil_activities_SRCS
    activityWidget.ui)

kde4_add_kcfg_files(kcm_powerdevil_activities_SRCS ../../PowerDevilSettings.kcfgc)

add_library(kcm_powerdevilactivitiesconfig MODULE ${kcm_powerdevil_activities_SRCS})

target_link_libraries(kcm_powerdevilactivitiesconfig
     
     ${KDE4_KDEUI_LIBRARY}
     KF5::Solid
     ${KACTIVITIES_LIBRARY}
     powerdevilconfigcommonprivate
)

install(TARGETS kcm_powerdevilactivitiesconfig  DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES powerdevilactivitiesconfig.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
