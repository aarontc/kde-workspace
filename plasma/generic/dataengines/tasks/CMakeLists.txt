set(tasks_engine_SRCS
    tasksengine.cpp
    tasksource.cpp
    taskservice.cpp
    taskjob.cpp
    virtualdesktopssource.cpp
     taskwindowservice.cpp
   taskwindowjob.cpp
)
kservice_desktop_to_json(plasma-dataengine-tasks.desktop)

add_library(plasma_engine_tasks MODULE ${tasks_engine_SRCS})
target_link_libraries(plasma_engine_tasks
                      KF5::KCoreAddons
                      KF5::KService
                      KF5::KWindowSystem
                      KF5::Plasma
                      Qt5::DBus
                      taskmanager)

install(TARGETS plasma_engine_tasks DESTINATION ${PLUGIN_INSTALL_DIR}/plasma/dataengine)
install(FILES plasma-dataengine-tasks.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES tasks.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
install(FILES windowtasks.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
