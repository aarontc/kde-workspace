include_directories( BEFORE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/backend )


install_pam_service(KDM)

include(ConfigureChecks.cmake)

install( FILES README DESTINATION ${DATA_INSTALL_DIR}/doc/kdm )

set(confproc ${CMAKE_CURRENT_SOURCE_DIR}/confproc.pl)
set(confdef ${CMAKE_CURRENT_SOURCE_DIR}/config.def)
set(confci ${CMAKE_CURRENT_BINARY_DIR}/config.ci)
macro_additional_clean_files( ${confci} )

add_custom_command(
	OUTPUT ${confci}
	COMMAND ${PERL_EXECUTABLE} -w ${confproc} ${confdef} ${confci}
	MAIN_DEPENDENCY ${confdef}
	DEPENDS ${confproc}
)

add_custom_target(ConfigCi DEPENDS ${confci})

### this is *only* for the kdm home page maintainer! ###
add_custom_target(hp scp
	${CMAKE_CURRENT_SOURCE_DIR}/README
	${CMAKE_CURRENT_SOURCE_DIR}/ChangeLog
	${CMAKE_CURRENT_SOURCE_DIR}/TODO
	devel-home:files/kdm)

set(backgroundlib_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/kcm/background/bgrender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kcm/background/bgsettings.cpp
)

# after confci is defined
add_subdirectory(backend)
add_subdirectory(kfrontend)
add_subdirectory(kcm)
