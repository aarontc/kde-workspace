remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_FROM_BYTEARRAY)

########### next target ###############

set(ksysguard_LIB_SRCS
   processes.cpp
   process.cpp
   processes_local_p.cpp
   processes_remote_p.cpp
   processes_base_p.cpp
   processes_atop_p.cpp
)



add_library(processcore SHARED ${ksysguard_LIB_SRCS})

target_link_libraries(processcore
   PUBLIC
    ${Qt5Core_LIBRARIES}
    KF5::KI18n
   PRIVATE
    ${ZLIB_LIBRARIES}
)

if( ${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" )
  message(STATUS "Adding kvm library on NetBSD")
  target_link_libraries(processcore kvm)
endif()

set_target_properties(processcore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS processcore EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES processes.h process.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksysguard COMPONENT Devel)

#------ KAuth stuff

# Auth example helper
# set(ksysguardprocesslist_helper_srcs
#     helper.cpp
#     process.cpp
#     processes_local_p.cpp
#     processes_base_p.cpp)
#
# add_executable(ksysguardprocesslist_helper ${ksysguardprocesslist_helper_srcs})
# target_link_libraries(ksysguardprocesslist_helper )
# install(TARGETS ksysguardprocesslist_helper DESTINATION ${LIBEXEC_INSTALL_DIR})
#
# kde4_install_auth_helper_files(ksysguardprocesslist_helper org.kde.ksysguard.processlisthelper root)
# kde4_install_auth_actions(org.kde.ksysguard.processlisthelper actions.actions)
#
# set_target_properties(ksysguardprocesslist_helper PROPERTIES COMPILE_FLAGS "-Wall -ggdb")

