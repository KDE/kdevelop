
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9532 )

include_directories( ${KDevPlatform_SOURCE_DIR}/shell )

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

add_library(KDevPlatformTests ${kdevtests_LIB_SRCS})
add_library(KDev::Tests ALIAS KDevPlatformTests)

generate_export_header(KDevPlatformTests EXPORT_FILE_NAME kdevplatformtestsexport.h)
target_link_libraries(KDevPlatformTests LINK_PUBLIC
    KDev::Shell
    KDev::Language
    KDev::Project
)
target_link_libraries(KDevPlatformTests LINK_PRIVATE
    Qt5::Core
    Qt5::Test
)

set_target_properties(KDevPlatformTests PROPERTIES
    VERSION ${KDEVPLATFORM_LIB_VERSION}
    SOVERSION ${KDEVPLATFORM_LIB_SOVERSION}
    EXPORT_NAME Tests
)
install(TARGETS KDevPlatformTests EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

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
    ${CMAKE_CURRENT_BINARY_DIR}/kdevplatformtestsexport.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/tests/
    COMPONENT Devel
)

add_subdirectory(json)
