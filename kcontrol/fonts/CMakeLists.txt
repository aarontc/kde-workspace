if(FONTCONFIG_FOUND)
    include_directories(${FONTCONFIG_INCLUDE_DIR})
endif()
include_directories(${FREETYPE_INCLUDE_DIRS})

########### next target ###############

set(kcm_fonts_PART_SRCS ../krdb/krdb.cpp fonts.cpp)

if(Q_WS_X11)
    set(kcm_fonts_PART_SRCS ${kcm_fonts_PART_SRCS} ${libkxftconfig_SRCS})
endif()

set(klauncher_xml ${KDE4_DBUS_INTERFACES_DIR}/org.kde.KLauncher.xml)
qt5_add_dbus_interface(kcm_fonts_PART_SRCS ${klauncher_xml} klauncher_iface)

add_library(kcm_fonts MODULE ${kcm_fonts_PART_SRCS})


target_link_libraries(kcm_fonts Qt5::DBus Qt5::Xml KF5::KCMUtils KF5::KI18n KF5::KDE4Support ${FREETYPE_LIBRARIES})

if(Q_WS_X11)
    if(FONTCONFIG_FOUND)
        target_link_libraries(kcm_fonts ${FONTCONFIG_LIBRARIES})
    endif()

    target_link_libraries(kcm_fonts ${X11_LIBRARIES})
endif()

install(TARGETS kcm_fonts  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES fonts.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
