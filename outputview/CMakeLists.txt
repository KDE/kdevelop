add_definitions(-DTRANSLATION_DOMAIN=\"kdevplatform\")

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
add_library( KDevPlatformOutputView ${outputviewinterfaces_LIB_SRCS} )
add_library( KDev::OutputView ALIAS KDevPlatformOutputView )
generate_export_header(KDevPlatformOutputView EXPORT_FILE_NAME outputviewexport.h)
target_link_libraries(KDevPlatformOutputView PRIVATE
    Qt5::Core
    KDev::Interfaces
    KDev::Util
)
set_target_properties(KDevPlatformOutputView PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION} EXPORT_NAME OutputView)
install(TARGETS KDevPlatformOutputView EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

 ########### install files ###############
 install( FILES
          ioutputview.h
          filtereditem.h
          outputmodel.h
          outputdelegate.h
          ioutputviewmodel.h
          ifilterstrategy.h
          ${CMAKE_CURRENT_BINARY_DIR}/outputviewexport.h
          outputjob.h
          outputexecutejob.h
          DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/outputview COMPONENT Devel)

add_subdirectory(tests)
