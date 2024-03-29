
add_subdirectory( pics )

configure_file (config-kcontrol-input.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kcontrol-input.h )

include_directories( ${KDE4_INCLUDES} )


include_directories( ${LIBUSB_INCLUDE_DIR} )

if(X11_Xcursor_FOUND)
  set( libnoinst_SRCS
    xcursor/themepage.cpp
    xcursor/thememodel.cpp
    xcursor/cursortheme.cpp
    xcursor/xcursortheme.cpp
    xcursor/previewwidget.cpp
    xcursor/itemdelegate.cpp
    xcursor/sortproxymodel.cpp
    ../krdb/krdb.cpp )
  kde4_add_ui_files( libnoinst_SRCS xcursor/themepage.ui )
  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/xcursor/ )
else()
  set( libnoinst_SRCS core/themepage.cpp )
  include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/core/ )
endif()

set(klauncher_xml ${KDE4_DBUS_INTERFACES_DIR}/org.kde.KLauncher.xml)
qt5_add_dbus_interface(libnoinst_SRCS ${klauncher_xml} klauncher_iface)


########### next target ###############

set(kapplymousetheme_SRCS kapplymousetheme.cpp )


add_executable(kapplymousetheme ${kapplymousetheme_SRCS})

target_link_libraries(kapplymousetheme ${X11_Xrender_LIB} ${X11_X11_LIB})
if (X11_Xcursor_FOUND)
   target_link_libraries(kapplymousetheme ${X11_Xcursor_LIB})
endif ()

install(TARGETS kapplymousetheme ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(kcm_input_PART_SRCS mouse.cpp main.cpp logitechmouse.cpp)


kde4_add_ui_files(kcm_input_PART_SRCS kmousedlg.ui logitechmouse_base.ui )
qt5_add_dbus_interface(kcm_input_PART_SRCS ${klauncher_xml} klauncher_iface)

add_library(kcm_input MODULE ${kcm_input_PART_SRCS})


target_link_libraries(kcm_input
    Qt5::DBus
    Qt5::X11Extras
    KF5::KCMUtils
    KF5::KI18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::KDE4Support
    ${X11_LIBRARIES}
    ${LIBUSB_LIBRARIES}
    )
if (X11_Xcursor_FOUND)
   target_link_libraries(kcm_input ${X11_Xcursor_LIB})
endif ()
if (X11_Xfixes_FOUND)
   target_link_libraries(kcm_input ${X11_Xfixes_LIB})
endif ()

install(TARGETS kcm_input  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(kcm_cursortheme_PART_SRCS kcmcursortheme.cpp ${libnoinst_SRCS})

add_library(kcm_cursortheme MODULE ${kcm_cursortheme_PART_SRCS})


target_link_libraries(kcm_cursortheme
    Qt5::DBus
    Qt5::X11Extras
    KF5::KArchive
    KF5::KCMUtils
    KF5::KI18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::KDE4Support
    KF5::KNewStuff
    ${X11_LIBRARIES}
    ${XCB_XCB_LIBRARY}
)

if (X11_Xcursor_FOUND)
   target_link_libraries(kcm_cursortheme ${X11_Xcursor_LIB})
endif ()
if (X11_Xfixes_FOUND)
   target_link_libraries(kcm_cursortheme ${X11_Xfixes_LIB})
endif ()

install(TARGETS kcm_cursortheme  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES mouse.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES cursortheme.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
install( FILES cursor_large_black.pcf.gz cursor_large_white.pcf.gz cursor_small_white.pcf.gz  DESTINATION  ${DATA_INSTALL_DIR}/kcminput )
install( FILES xcursor/xcursor.knsrc DESTINATION  ${CONFIG_INSTALL_DIR} )

