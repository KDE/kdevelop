include_directories(
    ${CMAKE_SOURCE_DIR}/lib/
    ${CMAKE_SOURCE_DIR}/lib/interfaces
)

set(koncreteproject_LIB_SRCS
    projectmodel.cpp
    importprojectjob.cpp
)

kde4_automoc(${koncreteproject_LIB_SRCS})

kde4_add_library(koncreteproject SHARED ${koncreteproject_LIB_SRCS})
target_link_libraries(koncreteproject ${QT_QTDESIGNER_LIBRARY} koncreteinterfaces )
set_target_properties(koncreteproject PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS koncreteproject DESTINATION ${LIB_INSTALL_DIR} )

########### install files ###############

install(FILES
    interfaces/iprojectbuilder.h
    interfaces/ifilemanager.h
    interfaces/ibuildmanager.h
    importprojectjob.h
    projectmodel.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/project/interfaces
 )