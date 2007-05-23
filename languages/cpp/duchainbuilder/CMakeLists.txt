
add_subdirectory(tests)

include_directories(
    ${CMAKE_TOP_BUILD_DIR}
    ${CMAKE_SOURCE_DIR}/lib/
    ${CMAKE_SOURCE_DIR}/lib/interfaces
    ${CMAKE_SOURCE_DIR}/lib/language/interfaces
    ${CMAKE_SOURCE_DIR}/lib/language/duchain
    ${CMAKE_SOURCE_DIR}/lib/editor
    ${CMAKE_SOURCE_DIR}/languages/cpp
    ${CMAKE_SOURCE_DIR}/languages/cpp/parser
)

########### next target ###############

set(kdevcppduchainbuilder_LIB_SRCS
    contextbuilder.cpp
    declarationbuilder.cpp
    typebuilder.cpp
    usebuilder.cpp
    cppeditorintegrator.cpp
    dumpchain.cpp
    cpptypes.cpp
    typerepository.cpp
    dumptypes.cpp
)

kde4_add_library(kdevcppduchainbuilder SHARED ${kdevcppduchainbuilder_LIB_SRCS})
target_link_libraries(kdevcppduchainbuilder kdevduchain kdevcppparser)
set_target_properties(kdevcppduchainbuilder PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevcppduchainbuilder DESTINATION ${LIB_INSTALL_DIR} )

########### install files ###############

install(FILES
    contextbuilder.h
    cppeditorintegrator.h
    cpptypes.h
    declarationbuilder.h
    dumpchain.h
    typebuilder.h
    typerepository.h
    usebuilder.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/cpp/duchainbuilder
)
