


########### next target ###############

set(kcm_joystick_PART_SRCS 
   joystick.cpp 
   joywidget.cpp 
   poswidget.cpp 
   joydevice.cpp 
   caldialog.cpp )


add_library(kcm_joystick MODULE ${kcm_joystick_PART_SRCS})


target_link_libraries(kcm_joystick
    KF5::KCompletion
    KF5::KCMUtils
    KF5::KI18n
    KF5::KIOWidgets
    KF5::KDE4Support
)

install(TARGETS kcm_joystick  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES joystick.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

#
