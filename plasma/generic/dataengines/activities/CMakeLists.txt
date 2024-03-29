project(activityengine)

set(activity_engine_SRCS
        ActivityData.cpp
        activityengine.cpp
        activityservice.cpp
        activityjob.cpp)

set_source_files_properties(org.kde.ActivityManager.ActivityRanking.xml PROPERTIES INCLUDE "ActivityData.h")
qt5_add_dbus_interface(
   activity_engine_SRCS org.kde.ActivityManager.ActivityRanking.xml
   ActivityRankingInterface
   )

kservice_desktop_to_json(plasma-engine-activities.desktop)
add_library(plasma_engine_activities MODULE ${activity_engine_SRCS})
target_link_libraries(plasma_engine_activities
                      KF5::KCoreAddons
                      KF5::Plasma
                      KF5::KActivities
                      KF5::KI18n
                      KF5::KService
                      Qt5::DBus
                      )

install(TARGETS plasma_engine_activities
        DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)

install(FILES plasma-engine-activities.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES activities.operations
        DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
