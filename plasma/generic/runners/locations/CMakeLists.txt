set(krunner_locations_SRCS
    locationrunner.cpp
)

add_library(krunner_locations MODULE ${krunner_locations_SRCS})
target_link_libraries(krunner_locations KIOCore KF5::Plasma)

install(TARGETS krunner_locations DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES plasma-runner-locations.desktop DESTINATION ${SERVICES_INSTALL_DIR})

