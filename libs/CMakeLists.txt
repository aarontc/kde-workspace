#add_subdirectory(kephal)
add_subdirectory(kworkspace)
add_subdirectory(oxygen)
#add_subdirectory(plasmaclock)

if(${KDE_PLATFORM_PROFILE} STREQUAL "Desktop")
   #add_subdirectory(kdm)
   #add_subdirectory(plasmagenericshell)
   if(NOT WIN32)
        add_subdirectory(taskmanager)
   endif()
endif()

if(NOT WIN32)
    if (NOT CMAKE_SYSTEM_NAME MATCHES Darwin)
        add_subdirectory(ksysguard)
    endif ()
endif()

