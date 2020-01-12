add_definitions(-DTRANSLATION_DOMAIN=\"kdevcompileanalyzercommon\")

set(KDevCompileAnalyzerCommon_SRCS
    compileanalyzejob.cpp
    compileanalyzeproblemmodel.cpp
    compileanalyzeutils.cpp
    compileanalyzer.cpp
)
declare_qt_logging_category(KDevCompileAnalyzerCommon_SRCS
    TYPE PLUGIN # TODO: need type private library
    IDENTIFIER KDEV_COMPILEANALYZER
    CATEGORY_BASENAME "compileanalyzer"
)

kdevelop_add_private_library(KDevCompileAnalyzerCommon SOURCES ${KDevCompileAnalyzerCommon_SRCS})
target_link_libraries(KDevCompileAnalyzerCommon
    PUBLIC
        KDev::OutputView
        KDev::Shell
        KDev::Project
        KDev::Util
    PRIVATE
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
