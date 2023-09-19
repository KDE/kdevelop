add_definitions(-DTRANSLATION_DOMAIN=\"kdevplatform\")

set(outputviewinterfaces_LIB_SRCS
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
declare_qt_logging_category(outputviewinterfaces_LIB_SRCS
    TYPE LIBRARY
    CATEGORY_BASENAME "outputview"
)
kdevplatform_add_library(KDevPlatformOutputView SOURCES ${outputviewinterfaces_LIB_SRCS})
target_link_libraries(KDevPlatformOutputView
PUBLIC
    KF6::CoreAddons
    Qt::Widgets
PRIVATE
    KDev::Interfaces
    KDev::Util
)

install(FILES
    ioutputview.h
    filtereditem.h
    outputmodel.h
    outputdelegate.h
    outputfilteringstrategies.h
    ioutputviewmodel.h
    ifilterstrategy.h
    outputjob.h
    outputexecutejob.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/outputview COMPONENT Devel)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
