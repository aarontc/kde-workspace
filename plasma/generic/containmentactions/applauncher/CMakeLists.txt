project(plasma-containmentactions-applauncher)

set(applauncher_SRCS
    launch.cpp
)

kservice_desktop_to_json(plasma-containmentactions-applauncher.desktop)

add_library(plasma_containmentactions_applauncher MODULE ${applauncher_SRCS})
target_link_libraries(plasma_containmentactions_applauncher KF5::Plasma KF5::KIOCore KF5::KIOWidgets)

install(TARGETS plasma_containmentactions_applauncher DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containmentactions-applauncher.desktop DESTINATION ${SERVICES_INSTALL_DIR})
