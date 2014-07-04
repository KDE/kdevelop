include_directories(${KDevPlatform_SOURCE_DIR}/shell ${QJSON_INCLUDE_DIR})

set(kdevplatformjsontests_LIB_SRCS
    delayedoutput.cpp
    declarationvalidator.cpp
    testsuite.cpp
)

add_library(kdevplatformjsontests SHARED ${kdevplatformjsontests_LIB_SRCS})
generate_export_header(kdevplatformjsontests EXPORT_FILE_NAME kdevplatformjsontestsexport.h)
target_link_libraries(kdevplatformjsontests
  LINK_PUBLIC
    Qt5::Core
    kdevplatformlanguage
)
set_target_properties(kdevplatformjsontests PROPERTIES
    VERSION ${KDEVPLATFORM_LIB_VERSION}
    SOVERSION ${KDEVPLATFORM_LIB_SOVERSION}
)

install(TARGETS kdevplatformjsontests EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES
    declarationvalidator.h
    delayedoutput.h
    jsontesthelpers.h
    testsuite.h
    testsuite.cpp
    jsondeclarationtests.h
    jsonducontexttests.h
    jsontypetests.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdevplatformjsontestsexport.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/tests/json/
    COMPONENT Devel
)
