set(kdevtests_LIB_SRCS
    autotestshell.cpp
    kdevsignalspy.cpp
    plugintesthelpers.cpp
    projectsgenerator.cpp
    testcore.cpp
    testproject.cpp
    testfile.cpp
    testlanguagecontroller.cpp
    testhelpers.cpp
    testplugincontroller.cpp
    testdebugsession.cpp
    testvariablecontroller.cpp
    testbreakpointcontroller.cpp
    testframestackmodel.cpp

    json/delayedoutput.cpp
    json/declarationvalidator.cpp
    json/testsuite.cpp
)

kdevplatform_add_library(KDevPlatformTests SOURCES ${kdevtests_LIB_SRCS})
target_link_libraries(KDevPlatformTests
PUBLIC
    KDev::Shell
    KDev::Language
    KDev::Project
    KDev::Debugger
PRIVATE
    Qt::Test
)

install(FILES
    autotestshell.h
    kdevsignalspy.h
    plugintesthelpers.h
    projectsgenerator.h
    testcore.h
    testproject.h
    testfile.h
    testlanguagecontroller.h
    testhelpers.h
    testplugincontroller.h
    testdebugsession.h
    testvariablecontroller.h
    testbreakpointcontroller.h
    testframestackmodel.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/tests/
    COMPONENT Devel
)

install(FILES
    json/declarationvalidator.h
    json/delayedoutput.h
    json/jsontesthelpers.h
    json/testsuite.h
    json/testsuite.cpp
    json/jsondeclarationtests.h
    json/jsonducontexttests.h
    json/jsontypetests.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/tests/json
    COMPONENT Devel
)
