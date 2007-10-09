include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(kdevplatformvcs_LIB_SRCS
    vcsjob.cpp
    vcsrevision.cpp
    vcsannotation.cpp
    vcslocation.cpp
    vcsdiff.cpp
    vcsevent.cpp
    vcsmapping.cpp
    vcsstatusinfo.cpp
)


kde4_add_library(kdevplatformvcs SHARED ${kdevplatformvcs_LIB_SRCS})
target_link_libraries(kdevplatformvcs ${KDE4_KDECORE_LIBS}  )
set_target_properties(kdevplatformvcs PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kdevplatformvcs DESTINATION ${LIB_INSTALL_DIR} )

install(FILES
    vcsexport.h
    vcsjob.h
    vcsrevision.h
    vcsannotation.h
    vcsdiff.h
    vcsevent.h
    vcsstatusinfo.h
    vcsmapping.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/vcs
)


install(FILES
    interfaces/ibasicversioncontrol.h
    interfaces/ibranchingversioncontrol.h
    interfaces/ibrowsableversioncontrol.h
    interfaces/idistributedversioncontrol.h
    interfaces/irepositoryversioncontrol.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/vcs/interfaces
)
