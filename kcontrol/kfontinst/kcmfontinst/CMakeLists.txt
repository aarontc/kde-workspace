include_directories( 
	${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/kfontinst/viewpart/
	 )

set(kcm_fontinst_PART_SRCS ${libkfontinstview_SRCS} ${libkfontinstjobrunner_SRCS} ${libkfontinstdbusiface_SRCS}
    DuplicatesDialog.cpp FontList.cpp GroupList.cpp KCmFontInst.cpp PrintDialog.cpp
    FontFilter.cpp FontFilterProxyStyle.cpp FcQuery.cpp PreviewList.cpp)
# qt5_add_dbus_interface(kcm_fontinst_PART_SRCS ../dbus/org.kde.fontinst.xml FontinstIface)

add_library(kcm_fontinst MODULE ${kcm_fontinst_PART_SRCS})

target_link_libraries(kcm_fontinst
    KF5::KArchive
    KF5::KCMUtils
    KF5::KDESu
    KF5::KIconThemes
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::XmlGui
    KF5::KNewStuff
    KF5::KDE4Support
    kfontinstui
    kfontinst
    ${X11_LIBRARIES}
)

install(TARGETS kcm_fontinst  DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES fontinst.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES kfontinst.knsrc DESTINATION  ${CONFIG_INSTALL_DIR} )

ecm_install_icons( ${DATA_INSTALL_DIR}/kfontinst/icons )
