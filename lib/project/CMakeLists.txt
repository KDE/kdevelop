include_directories(
    ${CMAKE_SOURCE_DIR}/lib/
    ${CMAKE_SOURCE_DIR}/lib/interfaces
)

set(kdevplatformproject_LIB_SRCS
    projectmodel.cpp
    importprojectjob.cpp
)

kde4_automoc(${kdevplatformproject_LIB_SRCS})

kde4_add_library(kdevplatformproject SHARED ${kdevplatformproject_LIB_SRCS})
target_link_libraries(kdevplatformproject ${QT_QTDESIGNER_LIBRARY} kdevplatforminterfaces )
set_target_properties(kdevplatformproject PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevplatformproject DESTINATION ${LIB_INSTALL_DIR} )

########### install files ###############

install(FILES
    interfaces/iprojectbuilder.h
    interfaces/iprojectfilemanager.h
    interfaces/ibuildsystemmanager.h
    importprojectjob.h
    projectmodel.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/project/interfaces
 )