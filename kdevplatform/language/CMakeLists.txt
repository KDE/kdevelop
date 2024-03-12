add_definitions(-DTRANSLATION_DOMAIN=\"kdevplatform\")

# Check whether malloc_trim(3) is supported.
include(CheckIncludeFile)
include(CheckSymbolExists)
check_include_file("malloc.h" HAVE_MALLOC_H)
check_symbol_exists(malloc_trim "malloc.h" HAVE_MALLOC_TRIM)

if(BUILD_TESTING)
    add_subdirectory(highlighting/tests)
    add_subdirectory(duchain/tests)
    add_subdirectory(backgroundparser/tests)
    add_subdirectory(codegen/tests)
    add_subdirectory(util/tests)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/language-features.h.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/language-features.h )

set(KDevPlatformLanguage_LIB_SRCS
    assistant/staticassistantsmanager.cpp
    assistant/renameaction.cpp
    assistant/renameassistant.cpp
    assistant/renamefileaction.cpp
    assistant/staticassistant.cpp

    editor/persistentmovingrangeprivate.cpp
    editor/persistentmovingrange.cpp
    editor/modificationrevisionset.cpp
    editor/modificationrevision.cpp

    backgroundparser/backgroundparser.cpp
    backgroundparser/parsejob.cpp
    backgroundparser/documentchangetracker.cpp
    backgroundparser/parseprojectjob.cpp
    backgroundparser/urlparselock.cpp

    duchain/specializationstore.cpp
    duchain/codemodel.cpp
    duchain/duchain.cpp
    duchain/waitforupdate.cpp
    duchain/duchainpointer.cpp
    duchain/ducontext.cpp
    duchain/indexedducontext.cpp
    duchain/indexedtopducontext.cpp
    duchain/localindexedducontext.cpp
    duchain/indexeddeclaration.cpp
    duchain/localindexeddeclaration.cpp
    duchain/topducontext.cpp
    duchain/topducontextdynamicdata.cpp
    duchain/topducontextutils.cpp
    duchain/functiondefinition.cpp
    duchain/declaration.cpp
    duchain/classmemberdeclaration.cpp
    duchain/classfunctiondeclaration.cpp
    duchain/classdeclaration.cpp
    duchain/use.cpp
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
    duchain/duchainutils.cpp
    duchain/declarationid.cpp
    duchain/definitions.cpp
    duchain/uses.cpp
    duchain/importers.cpp
    duchain/duchaindumper.cpp
    duchain/duchainregister.cpp
    duchain/persistentsymboltable.cpp
    duchain/instantiationinformation.cpp
    duchain/problem.cpp

    duchain/types/typesystem.cpp
    duchain/types/typeregister.cpp
    duchain/types/typerepository.cpp
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
    duchain/types/typeutils.cpp
    duchain/types/typealiastype.cpp
    duchain/types/unsuretype.cpp
    duchain/types/containertypes.cpp

    duchain/builders/dynamiclanguageexpressionvisitor.cpp

    duchain/navigation/problemnavigationcontext.cpp
    duchain/navigation/abstractnavigationwidget.cpp
    duchain/navigation/abstractnavigationcontext.cpp
    duchain/navigation/usesnavigationcontext.cpp
    duchain/navigation/abstractdeclarationnavigationcontext.cpp
    duchain/navigation/abstractincludenavigationcontext.cpp
    duchain/navigation/useswidget.cpp
    duchain/navigation/usescollector.cpp
    duchain/navigation/quickopenembeddedwidgetcombiner.cpp

    interfaces/abbreviations.cpp
    interfaces/iastcontainer.cpp
    interfaces/ilanguagesupport.cpp
    interfaces/quickopendataprovider.cpp
    interfaces/iquickopen.cpp
    interfaces/editorcontext.cpp
    interfaces/codecontext.cpp
    interfaces/icreateclasshelper.cpp
    interfaces/icontextbrowser.cpp

    codecompletion/codecompletion.cpp
    codecompletion/codecompletionworker.cpp
    codecompletion/codecompletionmodel.cpp
    codecompletion/codecompletionitem.cpp
    codecompletion/codecompletioncontext.cpp
    codecompletion/codecompletionitemgrouper.cpp
    codecompletion/codecompletionhelper.cpp
    codecompletion/normaldeclarationcompletionitem.cpp

    codegen/coderepresentation.cpp
    codegen/documentchangeset.cpp
    codegen/duchainchangeset.cpp
    codegen/utilities.cpp
    codegen/codedescription.cpp

    codegen/basicrefactoring.cpp
    codegen/progressdialogs/refactoringdialog.cpp

    util/setrepository.cpp
    util/includeitem.cpp
    util/navigationtooltip.cpp

    highlighting/colorcache.cpp
    highlighting/configurablecolors.cpp
    highlighting/codehighlighting.cpp
    highlighting/syntax/syntax.qrc

    checks/dataaccessrepository.cpp checks/dataaccess.cpp
    checks/controlflowgraph.cpp checks/controlflownode.cpp

    classmodel/classmodel.cpp
    classmodel/classmodelnode.cpp
    classmodel/classmodelnodescontroller.cpp
    classmodel/allclassesfolder.cpp
    classmodel/documentclassesfolder.cpp
    classmodel/projectfolder.cpp

    codegen/templatesmodel.cpp
    codegen/templatepreviewicon.cpp
    codegen/templateclassgenerator.cpp
    codegen/sourcefiletemplate.cpp
    codegen/templaterenderer.cpp
    codegen/templateengine.cpp
    codegen/archivetemplateloader.cpp
)

declare_qt_logging_category(KDevPlatformLanguage_LIB_SRCS
    TYPE LIBRARY
    CATEGORY_BASENAME "language"
)

ki18n_wrap_ui(KDevPlatformLanguage_LIB_SRCS
    codegen/basicrefactoring.ui
    codegen/progressdialogs/refactoringdialog.ui)

kdevplatform_add_library(KDevPlatformLanguage SOURCES ${KDevPlatformLanguage_LIB_SRCS})
target_include_directories(KDevPlatformLanguage PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(KDevPlatformLanguage
PUBLIC
    KDev::Serialization
    KDev::Interfaces
    KDev::Util
    KF6::ThreadWeaver
PRIVATE
    KDev::Project
    KDev::Sublime
    KF6::GuiAddons
    KF6::TextEditor
    KF6::Parts
    KF6::Archive
    KF6::TextTemplate
)

install(FILES
    assistant/renameaction.h
    assistant/renameassistant.h
    assistant/staticassistant.h
    assistant/staticassistantsmanager.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/language/assistant COMPONENT Devel
)

install(FILES
    interfaces/ilanguagesupport.h
    interfaces/icodehighlighting.h
    interfaces/quickopendataprovider.h
    interfaces/quickopenfilter.h
    interfaces/iquickopen.h
    interfaces/codecontext.h
    interfaces/editorcontext.h
    interfaces/iastcontainer.h
    interfaces/icreateclasshelper.h
    interfaces/icontextbrowser.h
    interfaces/abbreviations.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/language/interfaces COMPONENT Devel
)

install(FILES
    editor/persistentmovingrange.h
    editor/documentrange.h
    editor/documentcursor.h
    editor/cursorinrevision.h
    editor/rangeinrevision.h
    editor/modificationrevision.h
    editor/modificationrevisionset.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/language/editor COMPONENT Devel
)

install(FILES
    backgroundparser/backgroundparser.h
    backgroundparser/parsejob.h
    backgroundparser/parseprojectjob.h
    backgroundparser/urlparselock.h
    backgroundparser/documentchangetracker.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/language/backgroundparser COMPONENT Devel
)

install(FILES
    util/navigationtooltip.h
    util/setrepository.h
    util/basicsetrepository.h
    util/includeitem.h
    util/debuglanguageparserhelper.h
    util/kdevhash.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/language/util COMPONENT Devel
)

install(FILES
    duchain/parsingenvironment.h
    duchain/duchain.h
    duchain/codemodel.h
    duchain/ducontext.h
    duchain/ducontextdata.h
    duchain/topducontext.h
    duchain/topducontextutils.h
    duchain/topducontextdata.h
    duchain/declaration.h
    duchain/declarationdata.h
    duchain/classmemberdeclaration.h
    duchain/classmemberdeclarationdata.h
    duchain/classfunctiondeclaration.h
    duchain/classdeclaration.h
    duchain/functiondefinition.h
    duchain/use.h
    duchain/forwarddeclaration.h
    duchain/duchainbase.h
    duchain/duchainpointer.h
    duchain/duchainlock.h
    duchain/identifier.h
    duchain/abstractfunctiondeclaration.h
    duchain/functiondeclaration.h
    duchain/stringhelpers.h
    duchain/namespacealiasdeclaration.h
    duchain/aliasdeclaration.h
    duchain/dumpdotgraph.h
    duchain/duchainutils.h
    duchain/duchaindumper.h
    duchain/declarationid.h
    duchain/appendedlist.h
    duchain/duchainregister.h
    duchain/persistentsymboltable.h
    duchain/instantiationinformation.h
    duchain/specializationstore.h
    duchain/indexedducontext.h
    duchain/indexedtopducontext.h
    duchain/localindexedducontext.h
    duchain/indexeddeclaration.h
    duchain/localindexeddeclaration.h
    duchain/definitions.h
    duchain/problem.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/language/duchain COMPONENT Devel
)

install(FILES
    duchain/types/unsuretype.h
    duchain/types/identifiedtype.h
    duchain/types/typesystem.h
    duchain/types/typeregister.h
    duchain/types/typerepository.h
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
    duchain/types/typeutils.h
    duchain/types/typealiastype.h
    duchain/types/containertypes.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/language/duchain/types COMPONENT Devel
)

install(FILES
    duchain/builders/abstractcontextbuilder.h
    duchain/builders/abstractdeclarationbuilder.h
    duchain/builders/abstracttypebuilder.h
    duchain/builders/abstractusebuilder.h
    duchain/builders/dynamiclanguageexpressionvisitor.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/language/duchain/builders COMPONENT Devel
)

install(FILES
    codecompletion/codecompletion.h
    codecompletion/codecompletionworker.h
    codecompletion/codecompletionmodel.h
    codecompletion/codecompletionitem.h
    codecompletion/codecompletioncontext.h
    codecompletion/codecompletionitemgrouper.h
    codecompletion/codecompletionhelper.h
    codecompletion/normaldeclarationcompletionitem.h
    codecompletion/abstractincludefilecompletionitem.h
    codecompletion/codecompletiontesthelper.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/language/codecompletion COMPONENT Devel
)

install(FILES
    codegen/astchangeset.h
    codegen/duchainchangeset.h
    codegen/documentchangeset.h
    codegen/coderepresentation.h
    codegen/utilities.h
    codegen/templatesmodel.h
    codegen/templatepreviewicon.h
    codegen/templaterenderer.h
    codegen/templateengine.h
    codegen/sourcefiletemplate.h
    codegen/templateclassgenerator.h
    codegen/codedescription.h
    codegen/basicrefactoring.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/language/codegen COMPONENT Devel
)

install(FILES
    duchain/navigation/usesnavigationcontext.h
    duchain/navigation/abstractnavigationcontext.h
    duchain/navigation/abstractdeclarationnavigationcontext.h
    duchain/navigation/abstractincludenavigationcontext.h
    duchain/navigation/abstractnavigationwidget.h
    duchain/navigation/navigationaction.h
    duchain/navigation/useswidget.h
    duchain/navigation/usescollector.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/language/duchain/navigation COMPONENT Devel
)

install(FILES
    highlighting/codehighlighting.h
    highlighting/colorcache.h
    highlighting/configurablecolors.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/language/highlighting COMPONENT Devel
)

install(FILES
    checks/dataaccess.h
    checks/dataaccessrepository.h
    checks/controlflowgraph.h
    checks/controlflownode.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/language/checks COMPONENT Devel
)

install(FILES
    classmodel/classmodel.h
    classmodel/classmodelnode.h
    classmodel/classmodelnodescontroller.h
    classmodel/allclassesfolder.h
    classmodel/documentclassesfolder.h
    classmodel/projectfolder.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/language/classmodel COMPONENT Devel
)
