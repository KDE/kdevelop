set(KDevPlatformJsonTests_LIB_SRCS
    delayedoutput.cpp
    declarationvalidator.cpp
    testsuite.cpp
)

kdevplatform_add_library(KDevPlatformJsonTests SOURCES ${KDevPlatformJsonTests_LIB_SRCS})
target_link_libraries(KDevPlatformJsonTests
  LINK_PUBLIC
    Qt5::Core
    KDev::Language
)

# FIXME: Temporary hack
target_include_directories(KDevPlatformJsonTests INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/kdevplatform/jsontests>")

install(FILES
    declarationvalidator.h
    delayedoutput.h
    jsontesthelpers.h
    testsuite.h
    testsuite.cpp
    jsondeclarationtests.h
    jsonducontexttests.h
    jsontypetests.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/tests/json
    COMPONENT Devel
)
