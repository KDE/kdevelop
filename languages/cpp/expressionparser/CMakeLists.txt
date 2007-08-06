
add_subdirectory(tests)

include_directories(
    ${CMAKE_TOP_BUILD_DIR}
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/languages/cpp
)

########### next target ###############

set(kdevcppexpressionparser_LIB_SRCS
    expressionparser.cpp
    expressionvisitor.cpp
    typeconversion.cpp
    overloadresolution.cpp
    viablefunctions.cpp
)

# Note: This library doesn't follow API/ABI/BC rules and shouldn't have a SOVERSION
#       Its only purpose is to support the plugin without needing to add all source files
#       to the plugin target
kde4_add_library(kdevcppexpressionparser SHARED ${kdevcppexpressionparser_LIB_SRCS})
target_link_libraries(kdevcppexpressionparser ${KDEVPLATFORM_LANGUAGE_LIBRARY} ${KDEVPLATFORM_EDITOR_LIBRARY} kdevcppparser kdevcppduchainbuilder)
install(TARGETS kdevcppexpressionparser DESTINATION ${LIB_INSTALL_DIR} )

########### install files ###############

# install(FILES
#     expressionparser.h
#     expressionvisitor.h
#     typeconversion.h
#     DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/cpp/expressionparser
#
# )
