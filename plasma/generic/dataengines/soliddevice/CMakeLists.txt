set(soliddevice_engine_SRCS
    soliddeviceengine.cpp
    devicesignalmapper.cpp
    devicesignalmapmanager.cpp
    hddtemp.cpp
    soliddeviceservice.cpp
    soliddevicejob.cpp
)

kservice_desktop_to_json(plasma-dataengine-soliddevice.desktop)

add_library(plasma_engine_soliddevice MODULE ${soliddevice_engine_SRCS})

target_link_libraries(plasma_engine_soliddevice
  Qt5::Network
  KF5::KI18n
  KF5::KIOCore
  KF5::Plasma
  KF5::Solid
  KF5::KDE4Support
)

install(TARGETS plasma_engine_soliddevice DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
install(FILES plasma-dataengine-soliddevice.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES soliddevice.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services )

