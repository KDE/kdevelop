
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9507 )
set( outputviewinterfaces_LIB_SRCS
    outputdelegate.cpp
    outputformats.cpp
    filtereditem.cpp
    ifilterstrategy.cpp
    outputmodel.cpp
    ioutputview.cpp
    ioutputviewmodel.cpp
    outputfilteringstrategies.cpp
    outputjob.cpp
    outputexecutejob.cpp
)
kde4_add_library( kdevplatformoutputview SHARED ${outputviewinterfaces_LIB_SRCS} )
target_link_libraries(kdevplatformoutputview LINK_PUBLIC
    ${KDE4_KDEUI_LIBS}
)
target_link_libraries(kdevplatformoutputview LINK_PRIVATE
    ${QT_QTCORE_LIBRARY}
    kdevplatforminterfaces
    kdevplatformutil
)
set_target_properties(kdevplatformoutputview PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})
install(TARGETS kdevplatformoutputview EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

 ########### install files ###############
 install( FILES
          ioutputview.h
          filtereditem.h
          outputmodel.h
          outputdelegate.h
          ioutputviewmodel.h
          ifilterstrategy.h
          outputviewexport.h
          outputjob.h
          outputexecutejob.h
          DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/outputview COMPONENT Devel)

add_subdirectory(tests)
