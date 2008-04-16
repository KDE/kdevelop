

include_directories(
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9503 )
set(kdevplatformproject_LIB_SRCS
    projectmodel.cpp
    projectconfigskeleton.cpp
    importprojectjob.cpp
    interfaces/iprojectbuilder.cpp
    interfaces/iprojectfilemanager.cpp
    interfaces/ibuildsystemmanager.cpp
)


kde4_add_library(kdevplatformproject SHARED ${kdevplatformproject_LIB_SRCS})
target_link_libraries(kdevplatformproject ${QT_QTDESIGNER_LIBRARY} kdevplatforminterfaces )
set_target_properties(kdevplatformproject PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kdevplatformproject ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install(FILES
    interfaces/iprojectbuilder.h
    interfaces/iprojectfilemanager.h
    interfaces/ibuildsystemmanager.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/project/interfaces
 )

install(FILES
    importprojectjob.h
    projectconfigskeleton.h
    projectmodel.h
    projectkcmodule.h
    projectexport.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/project
 )
