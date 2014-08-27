include_directories(${KDevPlatform_SOURCE_DIR}/shell ${QJSON_INCLUDE_DIR})

set(KDevPlatformJsonTests_LIB_SRCS
    delayedoutput.cpp
    declarationvalidator.cpp
    testsuite.cpp
)

add_library(KDevPlatformJsonTests ${KDevPlatformJsonTests_LIB_SRCS})
add_library(KDev::JsonTests ALIAS KDevPlatformJsonTests)

generate_export_header(KDevPlatformJsonTests EXPORT_FILE_NAME kdevplatformjsontestsexport.h)
target_link_libraries(KDevPlatformJsonTests
  LINK_PUBLIC
    Qt5::Core
    KDev::Language
)
set_target_properties(KDevPlatformJsonTests PROPERTIES
    VERSION ${KDEVPLATFORM_LIB_VERSION}
    SOVERSION ${KDEVPLATFORM_LIB_SOVERSION}
    EXPORT_NAME JsonTests
)

install(TARGETS KDevPlatformJsonTests EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

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
