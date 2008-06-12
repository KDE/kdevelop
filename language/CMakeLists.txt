
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9505 )

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/language/interfaces
    ${CMAKE_SOURCE_DIR}/language/duchain
    ${CMAKE_SOURCE_DIR}/language/backgroundparser
    ${CMAKE_SOURCE_DIR}/editor
    ${CMAKE_SOURCE_DIR}/util
)

set(kdevplatformlanguage_LIB_SRCS
    backgroundparser/backgroundparser.cpp
    backgroundparser/parsejob.cpp
    backgroundparser/parserdependencypolicy.cpp
    duchain/duchain.cpp
    duchain/duchainpointer.cpp
    duchain/ducontext.cpp
    duchain/topducontext.cpp
    duchain/declaration.cpp
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
    duchain/parsingenvironment.cpp
    duchain/abstractfunctiondeclaration.cpp
    duchain/functiondeclaration.cpp
    duchain/stringhelpers.cpp
    duchain/namespacealiasdeclaration.cpp
    duchain/dumpdotgraph.cpp
    duchain/duchainobserver.cpp
    duchain/duchainutils.cpp
    duchain/forwarddeclarationtype.cpp
    duchain/declarationid.cpp
    duchain/definitions.cpp
    duchain/uses.cpp
    duchain/dumpchain.cpp
    duchain/indexedstring.cpp
    duchain/repositories/itemrepository.cpp
    interfaces/ilanguagesupport.cpp
    interfaces/quickopendataprovider.cpp
    interfaces/iproblem.cpp
    interfaces/iquickopen.cpp
    codecompletion/codecompletion.cpp
    codecompletion/codecompletionworker.cpp
    codecompletion/codecompletionmodel.cpp
    codecompletion/codecompletionitem.cpp
    codecompletion/codecompletioncontext.cpp
)


kde4_add_library(kdevplatformlanguage SHARED ${kdevplatformlanguage_LIB_SRCS})
target_link_libraries(kdevplatformlanguage ${KDE4_KPARTS_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_THREADWEAVER_LIBRARIES}
    kdevplatforminterfaces kdevplatformeditor)
set_target_properties(kdevplatformlanguage PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kdevplatformlanguage ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install(FILES
    languageexport.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language
)

install(FILES
    interfaces/ilanguagesupport.h
    interfaces/icodehighlighting.h
    interfaces/quickopendataprovider.h
    interfaces/quickopenfilter.h
    interfaces/iquickopen.h
    interfaces/iproblem.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/interfaces
)

install(FILES
    backgroundparser/backgroundparser.h
    backgroundparser/parsejob.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/backgroundparser
)

install(FILES
    duchain/forwarddeclarationtype.h
    duchain/parsingenvironment.h
    duchain/duchain.h
    duchain/ducontext.h
    duchain/topducontext.h
    duchain/declaration.h
    duchain/classmemberdeclaration.h
    duchain/classfunctiondeclaration.h
    duchain/use.h
    duchain/typesystem.h
    duchain/smartconverter.h
    duchain/forwarddeclaration.h
    duchain/duchainbase.h
    duchain/duchainpointer.h
    duchain/duchainobserver.h
    duchain/duchainlock.h
    duchain/identifier.h
    duchain/identifiedtype.h
    duchain/abstractfunctiondeclaration.h
    duchain/functiondeclaration.h
    duchain/stringhelpers.h
    duchain/namespacealiasdeclaration.h
    duchain/dumpdotgraph.h
    duchain/duchainutils.h
    duchain/basecontextbuilder.h
    duchain/dumpchain.h
    duchain/symboltable.h
    duchain/indexedstring.h
    duchain/repositories/itemrepository.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/duchain
)

install(FILES
    codecompletion/codecompletion.h
    codecompletion/codecompletionworker.h
    codecompletion/codecompletionmodel.h
    codecompletion/codecompletionitem.h
    codecompletion/codecompletioncontext.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/codecompletion
)
