include_directories( ${KDevPlatform_SOURCE_DIR}/shell )

set(kdevplatformjsontests_LIB_SRCS
    delayedoutput.cpp
    declarationvalidator.cpp
)

kde4_add_library(kdevplatformjsontests SHARED ${kdevplatformjsontests_LIB_SRCS})
target_link_libraries(kdevplatformjsontests
    ${QT_QTCORE_LIBRARY}
    ${QJSON_LIBRARIES}
    ${qjson_LIBRARIES}
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
    kdevplatformjsontestsexport.h
    jsontesthelpers.h
    testsuite.h
    jsondeclarationtests.h
    jsonducontexttests.h
    jsontypetests.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/tests/json/
    COMPONENT Devel
)
