# include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/fonts
#                     ${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/kfontinst/kcmfontinst
#                     ${CMAKE_CURRENT_BINARY_DIR})

set(kio_fonts_PART_SRCS FontInstInterface.cpp KioFonts.cpp ${libkfontinstdbusiface_SRCS})
# qt5_add_dbus_interface(kio_fonts_PART_SRCS ../dbus/org.kde.fontinst.xml FontinstIface)
add_library(kio_fonts MODULE ${kio_fonts_PART_SRCS} ${KFI_FONTINST_AUTH_SRC} )
target_link_libraries(kio_fonts Qt5::DBus Qt5::X11Extras Qt5::Xml KF5::KArchive KF5::KIOCore KF5::KIOWidgets kfontinst )
install(TARGETS kio_fonts  DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES fonts.desktop  DESTINATION  ${DATA_INSTALL_DIR}/konqsidebartng/virtual_folders/services )
install( FILES fonts.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )
ecm_install_icons( ${ICON_INSTALL_DIR}  )
