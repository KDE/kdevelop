
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

kde4_add_library(kdevcppexpressionparser SHARED ${kdevcppexpressionparser_LIB_SRCS})
target_link_libraries(kdevcppexpressionparser ${KDEVPLATFORM_LANGUAGE_LIBRARY} ${KDEVPLATFORM_EDITOR_LIBRARY} kdevcppparser kdevcppduchainbuilder)
set_target_properties(kdevcppexpressionparser PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevcppexpressionparser DESTINATION ${LIB_INSTALL_DIR} )

########### install files ###############

install(FILES
    expressionparser.h
    expressionvisitor.h
    typeconversion.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/cpp/expressionparser

)
