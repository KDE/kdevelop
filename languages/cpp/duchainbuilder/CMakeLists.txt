
add_subdirectory(tests)

include_directories(
    ${CMAKE_TOP_BUILD_DIR}
    ${CMAKE_SOURCE_DIR}/languages/cpp
)

########### next target ###############

set(kdevcppduchainbuilder_LIB_SRCS
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
)
# Note: This library doesn't follow API/ABI/BC rules and shouldn't have a SOVERSION
#       Its only purpose is to support the plugin without needing to add all source files
#       to the plugin target
kde4_add_library(kdev4cppduchainbuilder SHARED ${kdevcppduchainbuilder_LIB_SRCS})
target_link_libraries(kdev4cppduchainbuilder ${KDEVPLATFORM_LANGUAGE_LIBRARY} ${KDEVPLATFORM_EDITOR_LIBRARY} kdev4cppparser)
install(TARGETS kdev4cppduchainbuilder DESTINATION ${LIB_INSTALL_DIR} )

########### install files ###############

# install(FILES
#     contextbuilder.h
#     cppeditorintegrator.h
#     cpptypes.h
#     declarationbuilder.h
#     dumpchain.h
#     typebuilder.h
#     typerepository.h
#     usebuilder.h
#     DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/cpp/duchainbuilder
# )
