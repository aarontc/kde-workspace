set(filebrowser_engine_SRCS
    filebrowserengine.cpp
)

kservice_desktop_to_json(plasma-dataengine-filebrowser.desktop)

add_library(plasma_engine_filebrowser MODULE ${filebrowser_engine_SRCS})
target_link_libraries(plasma_engine_filebrowser
    KF5::Plasma
    KF5::KService
    KF5::KIOCore
    KF5::KDE4Support
)

install(TARGETS plasma_engine_filebrowser DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
install(FILES plasma-dataengine-filebrowser.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

