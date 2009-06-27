
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9532 )

set(kdevplatformtests_LIB_SRCS
    kdevsignalspy.cpp
)

kde4_add_library(kdevplatformtests SHARED ${kdevplatformtests_LIB_SRCS})
target_link_libraries(kdevplatformtests 
        ${QT_QTCORE_LIBRARY})
target_link_libraries(kdevplatformtests LINK_INTERFACE_LIBRARIES 
        ${QT_QTCORE_LIBRARY})
set_target_properties(kdevplatformtests PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})
install(TARGETS kdevplatformtests EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES 
         autotestshell.h 
         kdevsignalspy.h
         kdevplatformtestsexport.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/tests/ 
         COMPONENT Devel )
