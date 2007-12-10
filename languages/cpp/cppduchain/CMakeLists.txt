########### next target ###############

add_subdirectory(tests)

include_directories(
    ${CMAKE_TOP_BUILD_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/languages/cpp
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
    macroset.cpp
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
)

# Note: This library doesn't follow API/ABI/BC rules and shouldn't have a SOVERSION
#       Its only purpose is to support the plugin without needing to add all source files
#       to the plugin target

kde4_add_library(kdev4cppduchain SHARED ${kdevcppduchain_LIB_SRCS})
target_link_libraries(kdev4cppduchain ${KDEVPLATFORM_LANGUAGE_LIBRARY} ${KDEVPLATFORM_INTERFACES_LIBRARY} ${KDEVPLATFORM_EDITOR_LIBRARY} ${KDE4_KDE3SUPPORT_LIBRARY} kdev4cppparser)
install(TARGETS kdev4cppduchain DESTINATION ${LIB_INSTALL_DIR})

########### install files ###############

# install(FILES
#     expressionparser.h
#     expressionvisitor.h
#     typeconversion.h
#     DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/cpp/expressionparser
#
# )
