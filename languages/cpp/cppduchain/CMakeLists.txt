########### next target ###############

#First remove the cpp language part default area, then set our own
remove_definitions( -DKDE_DEFAULT_DEBUG_AREA=9007 )
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9041 )

add_subdirectory(tests)

include_directories(
    ${cpp_SOURCE_DIR}
    ${cpp_BINARY_DIR}
)

########### next target ###############

set(kdevcppduchain_LIB_SRCS
    adlhelper.cpp 
    sourcemanipulation.cpp
    missingdeclarationtype.cpp
    missingdeclarationproblem.cpp
    missingdeclarationassistant.cpp
    contextbuilder.cpp
    declarationbuilder.cpp
    typebuilder.cpp
    usebuilder.cpp
    cppeditorintegrator.cpp
    dumpchain.cpp
    cpptypes.cpp
    ptrtomembertype.cpp
    dumptypes.cpp
    environmentmanager.cpp
    cppduchain.cpp
    templateparameterdeclaration.cpp
    qtfunctiondeclaration.cpp
    cppducontext.cpp
    typeutils.cpp
    templatedeclaration.cpp
    cpppreprocessenvironment.cpp
    expressionparser.cpp
    expressionvisitor.cpp
    typeconversion.cpp
    overloadresolution.cpp
    templateresolver.cpp
    viablefunctions.cpp
    overloadresolutionhelper.cpp
    name_visitor.cpp
    type_visitor.cpp
    builtinoperators.cpp
    expressionevaluationresult.cpp
    usedecoratorvisitor.cpp 
    navigation/navigationwidget.cpp
    navigation/declarationnavigationcontext.cpp
    navigation/includenavigationcontext.cpp
    navigation/macronavigationcontext.cpp
    
    controlflowgraphbuilder.cpp
)

# Note: This library doesn't follow API/ABI/BC rules and shouldn't have a SOVERSION
#       Its only purpose is to support the plugin without needing to add all source files
#       to the plugin target

kde4_add_library(kdev4cppduchain SHARED ${kdevcppduchain_LIB_SRCS})
target_link_libraries(kdev4cppduchain LINK_PRIVATE
		${KDEVPLATFORM_LANGUAGE_LIBRARIES} 
		${KDEVPLATFORM_INTERFACES_LIBRARIES} 
		${KDEVPLATFORM_UTIL_LIBRARIES} 
		kdev4cppparser 
		${KDE4_KTEXTEDITOR_LIBS} 
		kdev4cpprpp)
install(TARGETS kdev4cppduchain ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

# install(FILES
#     expressionparser.h
#     expressionvisitor.h
#     typeconversion.h
#     DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/cpp/expressionparser
#
# )
