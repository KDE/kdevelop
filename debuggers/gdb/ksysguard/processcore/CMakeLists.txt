
########### next target ###############

set(ksysguard_LIB_SRCS
   processes.cpp
   process.cpp
   processes_local_p.cpp
   processes_remote_p.cpp
   processes_base_p.cpp
)



kde4_add_library(kdev_processcore SHARED ${ksysguard_LIB_SRCS})

target_link_libraries(kdev_processcore ${KDE4_KDECORE_LIBS} )
if( ${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" )
  message(STATUS "Adding kvm library on NetBSD")
  target_link_libraries(kdev_processcore kvm)
endif( ${CMAKE_SYSTEM_NAME} MATCHES "NetBSD" )

set_target_properties(kdev_processcore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kdev_processcore EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

#install( FILES processes.h process.h DESTINATION ${INCLUDE_INSTALL_DIR}/ksysguard COMPONENT Devel)

