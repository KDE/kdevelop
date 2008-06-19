########### next target ###############

#First remove the cpp language part default area, then set our own
remove_definitions( -DKDE_DEFAULT_DEBUG_AREA=9007 )
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9041 )

add_subdirectory(tests)

include_directories(
    ${CMAKE_TOP_BUILD_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/languages/cpp
    ${CMAKE_BINARY_DIR}/languages/cpp
)

########### next target ###############

set(kdevcppduchain_LIB_SRCS
    contextbuilder.cpp
    declarationbuilder.cpp
    typebuilder.cpp
    usebuilder.cpp
    cppeditorintegrator.cpp
    dumpchain.cpp
    cpptypes.cpp
    typerepository.cpp
    dumptypes.cpp
    environmentmanager.cpp
    cachemanager.cpp
    hashedstringset.cpp
    cppduchain.cpp
    templateparameterdeclaration.cpp
    cppducontext.cpp
    typeutils.cpp
    templatedeclaration.cpp
    setrepository.cpp
    cpppreprocessenvironment.cpp
    expressionparser.cpp
    expressionvisitor.cpp
    typeconversion.cpp
    overloadresolution.cpp
    viablefunctions.cpp
    overloadresolutionhelper.cpp
    navigationwidget.cpp
    name_visitor.cpp
    type_visitor.cpp
    builtinoperators.cpp
)

# Note: This library doesn't follow API/ABI/BC rules and shouldn't have a SOVERSION
#       Its only purpose is to support the plugin without needing to add all source files
#       to the plugin target

kde4_add_library(kdev4cppduchain SHARED ${kdevcppduchain_LIB_SRCS})
target_link_libraries(kdev4cppduchain ${KDEVPLATFORM_LANGUAGE_LIBRARY} ${KDEVPLATFORM_INTERFACES_LIBRARY} kdev4cppparser)
install(TARGETS kdev4cppduchain ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

# install(FILES
#     expressionparser.h
#     expressionvisitor.h
#     typeconversion.h
#     DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/cpp/expressionparser
#
# )
