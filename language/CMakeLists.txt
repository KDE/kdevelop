
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9505 )

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/language/interfaces
    ${CMAKE_SOURCE_DIR}/language/duchain
    ${CMAKE_SOURCE_DIR}/language/backgroundparser
    ${CMAKE_SOURCE_DIR}/language/editor
    ${CMAKE_SOURCE_DIR}/util
)

set(kdevplatformlanguage_LIB_SRCS
    editor/documentrange.cpp
    editor/documentrangeobject.cpp
    editor/documentcursor.cpp
    editor/documentcursorobject.cpp
    editor/editorintegrator.cpp
    editor/editorintegratorstatic.cpp
    editor/hashedstring.cpp
    editor/modificationrevision.cpp
    backgroundparser/backgroundparser.cpp
    backgroundparser/parsejob.cpp
    backgroundparser/parserdependencypolicy.cpp
    backgroundparser/documentchangetracker.cpp
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
    duchain/aliasdeclaration.cpp
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
    duchain/repositories/ityperepository.cpp
    duchain/repositories/typerepository.cpp
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
    kdevplatforminterfaces)
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
    editor/documentrange.h
    editor/documentrangeobject.h
    editor/documentcursor.h
    editor/documentcursorobject.h
    editor/editorintegrator.h
    editor/hashedstring.h
    editor/simplecursor.h
    editor/simplerange.h
    editor/modificationrevision.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/editor
)

install(FILES
    backgroundparser/backgroundparser.h
    backgroundparser/parsejob.h
    backgroundparser/documentchangetracker.h
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
    duchain/aliasdeclaration.h
    duchain/dumpdotgraph.h
    duchain/duchainutils.h
    duchain/abstractcontextbuilder.h
    duchain/abstractdeclarationbuilder.h
    duchain/abstracttypebuilder.h
    duchain/abstractusebuilder.h
    duchain/dumpchain.h
    duchain/symboltable.h
    duchain/indexedstring.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/duchain
)

install(FILES
    duchain/repositories/itemrepository.h
    duchain/repositories/ityperepository.h
    duchain/repositories/typerepository.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/duchain/repositories
)

install(FILES
    codecompletion/codecompletion.h
    codecompletion/codecompletionworker.h
    codecompletion/codecompletionmodel.h
    codecompletion/codecompletionitem.h
    codecompletion/codecompletioncontext.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/codecompletion
)
