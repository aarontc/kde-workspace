
#kservice_desktop_to_json(plasma-dataengine-powermanagement.desktop)

set(powermanagement_engine_SRCS
    powermanagementengine.cpp
    powermanagementjob.cpp
    powermanagementservice.cpp
)

set(krunner_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/dbus/org.kde.krunner.App.xml)
qt5_add_dbus_interface(powermanagement_engine_SRCS ${krunner_xml} krunner_interface)

kservice_desktop_to_json(plasma-dataengine-powermanagement.desktop)

add_library(plasma_engine_powermanagement MODULE ${powermanagement_engine_SRCS})

target_link_libraries(plasma_engine_powermanagement
    KF5::Solid
    KF5::Plasma
    KF5::KIdleTime
    KF5::KCoreAddons
    KF5::KI18n
    KF5::KService
    Qt5::DBus
    kworkspace
)

install(TARGETS plasma_engine_powermanagement DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
install(FILES plasma-dataengine-powermanagement.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES powermanagementservice.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

