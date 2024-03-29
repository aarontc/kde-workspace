project(shareengine)

set(share_engine_SRCS
        shareprovider.cpp
        shareengine.cpp
        shareservice.cpp
        share_package.cpp)

add_library(plasma_engine_share MODULE ${share_engine_SRCS})
target_link_libraries(plasma_engine_share
                      KF5::KCore
                      KF5::Plasma
                      KIOCore
                      ${KDE4_KROSSCORE_LIBS}
                      KF5::KDE4Support
)

install(TARGETS plasma_engine_share
        DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)

install(FILES data/plasma-dataengine-share.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES data/plasma_shareprovider.desktop
        DESTINATION ${SERVICETYPES_INSTALL_DIR})

install(FILES share.operations
        DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

set(sharepackage_SRCS
        share_package.cpp
        plugin_share_package.cpp
)

add_subdirectory(backends)

plasma_add_plugin(plasma_packagestructure_share
        ${sharepackage_SRCS})
target_link_libraries(plasma_packagestructure_share
        KF5::Plasma)

install(TARGETS plasma_packagestructure_share
                 DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/packagestructure)
install(FILES data/plasma-packagestructure-share.desktop
               DESTINATION ${SERVICES_INSTALL_DIR})
