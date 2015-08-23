set(kdevtests_LIB_SRCS
    autotestshell.cpp
    kdevsignalspy.cpp
    testcore.cpp
    testproject.cpp
    testfile.cpp
    testlanguagecontroller.cpp
    testhelpers.cpp
    testplugincontroller.cpp
    modeltest.cpp
)

kdevplatform_add_library(KDevPlatformTests SOURCES ${kdevtests_LIB_SRCS})
target_link_libraries(KDevPlatformTests LINK_PUBLIC
    KDev::Shell
    KDev::Language
    KDev::Project
)
target_link_libraries(KDevPlatformTests LINK_PRIVATE
    Qt5::Core
    Qt5::Test
)

install(FILES
    autotestshell.h
    kdevsignalspy.h
    modeltest.h
    testcore.h
    testproject.h
    testfile.h
    testlanguagecontroller.h
    testhelpers.h
    testplugincontroller.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/tests/
    COMPONENT Devel
)

add_subdirectory(json)
