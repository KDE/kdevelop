
add_subdirectory(viewer)

include_directories(
    ${CMAKE_TOP_BUILD_DIR}
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/interfaces
    ${CMAKE_SOURCE_DIR}/lib/language
    ${CMAKE_SOURCE_DIR}/lib/language/interfaces
    ${CMAKE_SOURCE_DIR}/lib/editor
)

set(kdevduchain_LIB_SRCS
    duchain.cpp
    ducontext.cpp
    topducontext.cpp
    declaration.cpp
    definition.cpp
    classmemberdeclaration.cpp
    classfunctiondeclaration.cpp
    use.cpp
    typesystem.cpp
    smartconverter.cpp
    symboltable.cpp
    forwarddeclaration.cpp
    duchainbase.cpp
    duchainlock.cpp
    identifier.cpp
    identifiedtype.cpp
)

kde4_automoc(${kdevduchain_LIB_SRCS})
kde4_add_library(kdevduchain SHARED ${kdevduchain_LIB_SRCS})
target_link_libraries(kdevduchain kdevplatformeditor)
set_target_properties(kdevduchain PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevduchain DESTINATION ${LIB_INSTALL_DIR})

########### install files ###############

install(FILES
    duchain.h
    ducontext.h
    topducontext.h
    declaration.h
    definition.h
    classmemberdeclaration.h
    classfunctiondeclaration.h
    use.h
    typesystem.h
    smartconverter.h
    symboltable.h
    forwarddeclaration.h
    duchainbase.h
    duchainlock.h
    identifier.h
    identifiedtype.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/language/duchain
)
