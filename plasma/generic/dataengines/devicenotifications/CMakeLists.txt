
set(device_notifications_engine_SRCS
    devicenotificationsengine.cpp
)

qt4_add_dbus_adaptor( device_notifications_engine_SRCS org.kde.DeviceNotifications.xml devicenotificationsengine.h DeviceNotificationsEngine )

kservice_desktop_to_json(plasma-dataengine-devicenotifications.desktop)

add_library(plasma_engine_devicenotifications MODULE ${device_notifications_engine_SRCS})

target_link_libraries(plasma_engine_devicenotifications 
    Qt5::DBus
    KF5::KService
    KF5::Plasma
) # todo: add kworkspace once ported

install(TARGETS plasma_engine_devicenotifications DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
install(FILES plasma-dataengine-devicenotifications.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
