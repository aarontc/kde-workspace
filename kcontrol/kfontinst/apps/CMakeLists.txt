include_directories(
	${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/kfontinst/kcmfontinst/
	${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/kfontinst/viewpart/
	${CMAKE_CURRENT_BINARY_DIR}
	)

set(kfontinst_bin_SRCS ${libkfontinstjobrunner_SRCS} ${libkfontinstdbusiface_SRCS} Installer.cpp )
# qt5_add_dbus_interface(kfontinst_bin_SRCS ../dbus/org.kde.fontinst.xml FontInstInterfaceBase)

set(kfontprint_bin_SRCS ${libkfontinstactionlabel_SRCS} Printer.cpp )
set(kfontview_bin_SRCS Viewer.cpp )

add_executable(kfontinst_bin ${kfontinst_bin_SRCS})
add_executable(kfontprint_bin ${kfontprint_bin_SRCS})
add_executable(kfontview_bin ${kfontview_bin_SRCS})

set_target_properties(kfontinst_bin PROPERTIES OUTPUT_NAME kfontinst)
set_target_properties(kfontprint_bin PROPERTIES OUTPUT_NAME kfontprint)
set_target_properties(kfontview_bin PROPERTIES OUTPUT_NAME kfontview)

target_link_libraries(kfontinst_bin
    Qt5::X11Extras
    KF5::KArchive
    KF5::KIconThemes
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::XmlGui
    KF5::KDE4Support
    ${X11_X11_LIB}
    kfontinst
)
target_link_libraries(kfontprint_bin
    Qt5::PrintSupport
    KF5::KIconThemes
    KF5::KPrintUtils
    KF5::KDE4Support
    ${X11_X11_LIB}
    ${FREETYPE_LIBRARIES}
    kfontinstui
    kfontinst
)
target_link_libraries(kfontview_bin KF5::KParts KF5::XmlGui kfontinstui kfontinst )

install(TARGETS kfontinst_bin ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(TARGETS kfontprint_bin DESTINATION ${LIBEXEC_INSTALL_DIR} )
install(TARGETS kfontview_bin ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(FILES kfontviewui.rc DESTINATION ${DATA_INSTALL_DIR}/kfontview )
install(PROGRAMS kfontview.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install(FILES installfont.desktop DESTINATION
${SERVICES_INSTALL_DIR}/ServiceMenus )

ecm_install_icons( ${ICON_INSTALL_DIR} )
