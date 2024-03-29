
#######################################################################################
# Package
plasma_install_package(package org.kde.kickoff)


#######################################################################################
# Kickoff Library

set(libkickoff_SRCS
    core/applicationmodel.cpp
    core/favoritesmodel.cpp
    core/itemhandlers.cpp
    core/kickoffabstractmodel.cpp
    core/kickoffmodel.cpp
    core/kickoffproxymodel.cpp
    core/kickoffplugin.cpp
    core/krunnermodel.cpp
    core/leavemodel.cpp
    core/models.cpp
    core/recentapplications.cpp
    core/recentlyusedmodel.cpp
    core/systemmodel.cpp
    core/urlitemlauncher.cpp
)

qt4_automoc(${libkickoff_SRCS})

qt4_add_dbus_adaptor(libkickoff_SRCS core/org.kde.kickoff.xml core/applicationmodel.h Kickoff::ApplicationModel)
qt4_add_dbus_adaptor(libkickoff_SRCS core/org.kde.kickoff.recent.xml core/recentlyusedmodel.h Kickoff::RecentlyUsedModel)

set(screensaver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/screenlocker/dbus/org.freedesktop.ScreenSaver.xml)
qt5_add_dbus_interface(libkickoff_SRCS ${screensaver_xml} screensaver_interface)
set(krunner_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/dbus/org.kde.krunner.App.xml)
qt5_add_dbus_interface(libkickoff_SRCS ${krunner_xml} krunner_interface)
set(ksmserver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
qt5_add_dbus_interface(libkickoff_SRCS ${ksmserver_xml} ksmserver_interface)

install(FILES core/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/kickoff)
add_library(kickoffplugin SHARED ${libkickoff_SRCS})
target_link_libraries(kickoffplugin
                              Qt5::Core
                              Qt5::Qml
                              Qt5::DBus
                              KF5::KI18n
                              KF5::KIOCore
			      KF5::KIOWidgets
			      KF5::KIOFileWidgets
                              KF5::Plasma
                              KF5::Solid
                              KF5::KService
                              KF5::KDE4Support
                              kworkspace)
install(TARGETS kickoffplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/kickoff)

