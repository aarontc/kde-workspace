project(plasma-dictengine)

set(dict_engine_SRCS
    dictengine.cpp
)

kservice_desktop_to_json(plasma-dataengine-dict.desktop)

add_library(plasma_engine_dict MODULE ${dict_engine_SRCS})

target_link_libraries (plasma_engine_dict
  KF5::Plasma
  KF5::KService
  Qt5::Network
)

install(TARGETS plasma_engine_dict DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
install(FILES plasma-dataengine-dict.desktop DESTINATION ${SERVICES_INSTALL_DIR})