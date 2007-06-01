
include_directories(
    ${CMAKE_SOURCE_DIR}/lib/interfaces
    ${CMAKE_SOURCE_DIR}/lib/language/interfaces
    ${CMAKE_SOURCE_DIR}/lib/editor
)

set(kdevplatformlanguage_LIB_SRCS
    backgroundparser/backgroundparser.cpp
    backgroundparser/parsejob.cpp
    backgroundparser/parserdependencypolicy.cpp
    duchain/duchain.cpp
    duchain/ducontext.cpp
    duchain/topducontext.cpp
    duchain/declaration.cpp
    duchain/definition.cpp
    duchain/classmemberdeclaration.cpp
    duchain/classfunctiondeclaration.cpp
    duchain/use.cpp
    duchain/typesystem.cpp
    duchain/smartconverter.cpp
    duchain/symboltable.cpp
    duchain/forwarddeclaration.cpp
    duchain/duchainbase.cpp
    duchain/duchainlock.cpp
    duchain/identifier.cpp
    duchain/identifiedtype.cpp
)

kde4_automoc(${kdevplatformlanguage_LIB_SRCS})

kde4_add_library(kdevplatformlanguage SHARED ${kdevplatformlanguage_LIB_SRCS})
target_link_libraries(kdevplatformlanguage ${KDE4_KPARTS_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_THREADWEAVER_LIBRARIES}
    kdevplatforminterfaces kdevplatformeditor)
set_target_properties(kdevplatformlanguage PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevplatformlanguage DESTINATION ${LIB_INSTALL_DIR} )

########### install files ###############

install(FILES
    languageexport.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/language
)

install(FILES
    interfaces/ilanguagesupport.h
    interfaces/icodehighlighting.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/language/interfaces
)

install(FILES
    backgroundparser/backgroundparser.h
    backgroundparser/parsejob.h
    backgroundparser/parserdependencypolicy.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/language/backgroundparser
)

install(FILES
    duchain/duchain.h
    duchain/ducontext.h
    duchain/topducontext.h
    duchain/declaration.h
    duchain/definition.h
    duchain/classmemberdeclaration.h
    duchain/classfunctiondeclaration.h
    duchain/use.h
    duchain/typesystem.h
    duchain/smartconverter.h
    duchain/symboltable.h
    duchain/forwarddeclaration.h
    duchain/duchainbase.h
    duchain/duchainlock.h
    duchain/identifier.h
    duchain/identifiedtype.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/language/duchain
)
