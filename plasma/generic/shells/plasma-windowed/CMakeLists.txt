include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kworkspace)


set(plasma-windowed_SRCS
    singleview.cpp
    main.cpp
    plasmaapp.cpp
    ${appletbrowserdialog_SRCS}
)


kf5_add_kdeinit_executable(plasma-windowed ${plasma-windowed_SRCS})

target_link_libraries(kdeinit_plasma-windowed KF5::Plasma kworkspace KIOCore ${X11_LIBRARIES} KF5::KIOFileWidgets )


install(TARGETS kdeinit_plasma-windowed DESTINATION ${LIB_INSTALL_DIR})
install(TARGETS plasma-windowed ${INSTALL_TARGETS_DEFAULT_ARGS})

