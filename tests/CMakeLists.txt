
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9532 )

include_directories( ${KDevPlatform_SOURCE_DIR}/shell )

set(kdevplatformtests_LIB_SRCS
    kdevsignalspy.cpp
    testcore.cpp
    testproject.cpp
    testfile.cpp
    testlanguagecontroller.cpp
    testhelpers.cpp
    testplugincontroller.cpp
    modeltest.cpp
)

kde4_add_library(kdevplatformtests SHARED ${kdevplatformtests_LIB_SRCS})
target_link_libraries(kdevplatformtests 
        ${QT_QTCORE_LIBRARY}
        ${QT_QTTEST_LIBRARY}
        kdevplatformshell
        kdevplatformlanguage
        kdevplatformproject
)
target_link_libraries(kdevplatformtests LINK_INTERFACE_LIBRARIES
        kdevplatformshell
        kdevplatformlanguage
        kdevplatformproject
)

set_target_properties(kdevplatformtests PROPERTIES
    VERSION ${KDEVPLATFORM_LIB_VERSION}
    SOVERSION ${KDEVPLATFORM_LIB_SOVERSION}
)
install(TARGETS kdevplatformtests EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES
    autotestshell.h
    kdevsignalspy.h
    testcore.h
    testproject.h
    testfile.h
    testlanguagecontroller.h
    kdevplatformtestsexport.h
    testhelpers.h
    testplugincontroller.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/tests/
    COMPONENT Devel
)

if(QJSON_FOUND)
    add_subdirectory(json)
endif(QJSON_FOUND)
