
# add_subdirectory(tests)
add_subdirectory(viewer)

include_directories( ${CMAKE_TOP_BUILD_DIR}
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/interfaces
    ${CMAKE_SOURCE_DIR}/lib/language
    ${CMAKE_SOURCE_DIR}/lib/language/interfaces
    ${CMAKE_SOURCE_DIR}/lib/editor
    ${CMAKE_SOURCE_DIR}/languages/cpp/
    ${CMAKE_SOURCE_DIR}/languages/cpp/parser
)

set(kdevduchain_LIB_SRCS
    cppeditorintegrator.cpp
    duchain.cpp
    contextbuilder.cpp
    declarationbuilder.cpp
    usebuilder.cpp
    ducontext.cpp
    topducontext.cpp
    declaration.cpp
    definition.cpp
    ../kdevcodehighlighting.cpp
    classmemberdeclaration.cpp
    classfunctiondeclaration.cpp
    use.cpp
    dumpchain.cpp
    typesystem.cpp
    typebuilder.cpp
    cpptypes.cpp
    dumptypes.cpp
    typerepository.cpp
    smartconverter.cpp
    symboltable.cpp
    forwarddeclaration.cpp
    duchainbase.cpp
    duchainlock.cpp
)

kde4_automoc(${kdevduchain_LIB_SRCS})
kde4_add_library(kdevduchain SHARED ${kdevduchain_LIB_SRCS})
target_link_libraries(kdevduchain kdevplatformeditor kdevcppparser)
set_target_properties(kdevduchain PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevduchain DESTINATION ${LIB_INSTALL_DIR} )
