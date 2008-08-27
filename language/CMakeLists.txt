
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9505 )

set(kdevplatformlanguage_LIB_SRCS
    editor/documentrange.cpp
    editor/documentrangeobject.cpp
    editor/documentcursor.cpp
    editor/documentcursorobject.cpp
    editor/editorintegrator.cpp
    editor/editorintegratorstatic.cpp
    editor/hashedstring.cpp
    editor/modificationrevision.cpp
    editor/editorrevisiontracker.cpp
    backgroundparser/backgroundparser.cpp
    backgroundparser/parsejob.cpp
    backgroundparser/parserdependencypolicy.cpp
    backgroundparser/documentchangetracker.cpp
    duchain/codemodel.cpp
    duchain/duchain.cpp
    duchain/duchainpointer.cpp
    duchain/ducontext.cpp
    duchain/topducontext.cpp
    duchain/topducontextdynamicdata.cpp
    duchain/functiondefinition.cpp
    duchain/declaration.cpp
    duchain/classmemberdeclaration.cpp
    duchain/classfunctiondeclaration.cpp
    duchain/use.cpp
    duchain/smartconverter.cpp
    duchain/symboltable.cpp
    duchain/forwarddeclaration.cpp
    duchain/duchainbase.cpp
    duchain/duchainlock.cpp
    duchain/identifier.cpp
    duchain/parsingenvironment.cpp
    duchain/abstractfunctiondeclaration.cpp
    duchain/functiondeclaration.cpp
    duchain/stringhelpers.cpp
    duchain/namespacealiasdeclaration.cpp
    duchain/aliasdeclaration.cpp
    duchain/dumpdotgraph.cpp
    duchain/duchainobserver.cpp
    duchain/duchainutils.cpp
    duchain/declarationid.cpp
    duchain/definitions.cpp
    duchain/uses.cpp
    duchain/dumpchain.cpp
    duchain/indexedstring.cpp
    duchain/duchainregister.cpp
    duchain/persistentsymboltable.cpp

    duchain/types/typesystem.cpp
    duchain/types/typeregister.cpp
    duchain/types/identifiedtype.cpp
    duchain/types/abstracttype.cpp
    duchain/types/integraltype.cpp
    duchain/types/functiontype.cpp
    duchain/types/structuretype.cpp
    duchain/types/pointertype.cpp
    duchain/types/referencetype.cpp
    duchain/types/delayedtype.cpp
    duchain/types/arraytype.cpp
    duchain/types/indexedtype.cpp
    duchain/types/enumerationtype.cpp
    duchain/types/constantintegraltype.cpp
    duchain/types/enumeratortype.cpp

    duchain/repositories/itemrepository.cpp
    duchain/repositories/typerepository.cpp

    interfaces/ilanguagesupport.cpp
    interfaces/quickopendataprovider.cpp
    interfaces/iproblem.cpp
    interfaces/iquickopen.cpp
    interfaces/codecontext.cpp

    codecompletion/codecompletion.cpp
    codecompletion/codecompletionworker.cpp
    codecompletion/codecompletionmodel.cpp
    codecompletion/codecompletionitem.cpp
    codecompletion/codecompletioncontext.cpp

    codegen/createclass.cpp
)

set( kdevplatformlanguage_LIB_UI
    codegen/ui/newclass.ui
    codegen/ui/overridevirtuals.ui
    codegen/ui/licensechooser.ui
    codegen/ui/outputlocation.ui
)

kde4_add_ui_files( kdevplatformlanguage_LIB_SRCS ${kdevplatformlanguage_LIB_UI} )


kde4_add_library(kdevplatformlanguage SHARED ${kdevplatformlanguage_LIB_SRCS})
target_link_libraries(kdevplatformlanguage ${KDE4_KPARTS_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_THREADWEAVER_LIBRARIES}
    kdevplatforminterfaces)
set_target_properties(kdevplatformlanguage PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})
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
    interfaces/codecontext.h
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
    editor/editorrevisiontracker.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/editor
)

install(FILES
    backgroundparser/backgroundparser.h
    backgroundparser/parsejob.h
    backgroundparser/documentchangetracker.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/backgroundparser
)

install(FILES
    duchain/parsingenvironment.h
    duchain/duchain.h
    duchain/codemodel.h
    duchain/ducontext.h
    duchain/ducontextdata.h
    duchain/arrayhelpers.h
    duchain/topducontext.h
    duchain/topducontextdata.h
    duchain/declaration.h
    duchain/declarationdata.h
    duchain/classmemberdeclaration.h
    duchain/classmemberdeclarationdata.h
    duchain/classfunctiondeclaration.h
    duchain/functiondefinition.h
    duchain/use.h
    duchain/smartconverter.h
    duchain/forwarddeclaration.h
    duchain/duchainbase.h
    duchain/duchainpointer.h
    duchain/duchainobserver.h
    duchain/duchainlock.h
    duchain/identifier.h
    duchain/abstractfunctiondeclaration.h
    duchain/functiondeclaration.h
    duchain/stringhelpers.h
    duchain/safetycounter.h
    duchain/namespacealiasdeclaration.h
    duchain/aliasdeclaration.h
    duchain/dumpdotgraph.h
    duchain/duchainutils.h
    duchain/dumpchain.h
    duchain/symboltable.h
    duchain/indexedstring.h
    duchain/declarationid.h
    duchain/appendedlist.h
    duchain/duchainregister.h
    duchain/persistentsymboltable.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/duchain
)

install(FILES
    duchain/types/identifiedtype.h
    duchain/types/typesystem.h
    duchain/types/typeregister.h
    duchain/types/typepointer.h
    duchain/types/typesystemdata.h
    duchain/types/abstracttype.h
    duchain/types/integraltype.h
    duchain/types/functiontype.h
    duchain/types/structuretype.h
    duchain/types/pointertype.h
    duchain/types/referencetype.h
    duchain/types/delayedtype.h
    duchain/types/arraytype.h
    duchain/types/indexedtype.h
    duchain/types/enumerationtype.h
    duchain/types/constantintegraltype.h
    duchain/types/enumeratortype.h
    duchain/types/alltypes.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/duchain/types
)

install(FILES
    duchain/builders/abstractcontextbuilder.h
    duchain/builders/abstractdeclarationbuilder.h
    duchain/builders/abstracttypebuilder.h
    duchain/builders/abstractusebuilder.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/duchain/builders
)

install(FILES
    duchain/repositories/itemrepository.h
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

install(FILES
    codegen/createclass.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/language/codegen
)
