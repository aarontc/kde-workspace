
set(time_engine_SRCS
    timeengine.cpp
    timesource.cpp
    solarsystem.cpp
)

kservice_desktop_to_json(plasma-dataengine-time.desktop)

add_library(plasma_engine_time MODULE ${time_engine_SRCS})

target_link_libraries(plasma_engine_time
    Qt5::DBus
    KF5::Solid
    KF5::Plasma
    KF5::KDE4Support
    KF5::KI18n
    KF5::KService
)

install(TARGETS plasma_engine_time DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
install(FILES plasma-dataengine-time.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
