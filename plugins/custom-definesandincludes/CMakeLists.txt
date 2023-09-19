add_definitions(-DTRANSLATION_DOMAIN=\"kdevcustomdefinesandincludes\")

declare_qt_logging_category(kdevdefinesandincludesmanager_LOG_SRCS
    TYPE PLUGIN
    IDENTIFIER DEFINESANDINCLUDES
    CATEGORY_BASENAME "definesandincludes"
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
add_subdirectory(compilerprovider)
add_subdirectory(noprojectincludesanddefines)

set( kdevdefinesandincludesmanager_SRCS
        definesandincludesmanager.cpp
        kcm_widget/projectpathsmodel.cpp
        kcm_widget/definesmodel.cpp
        kcm_widget/includesmodel.cpp
        kcm_widget/includeswidget.cpp
        kcm_widget/defineswidget.cpp
        kcm_widget/projectpathswidget.cpp
        kcm_widget/definesandincludesconfigpage.cpp
        kcm_widget/parserwidget.cpp
        compilerprovider/icompiler.cpp # TODO: is this really necessary
    )

ki18n_wrap_ui(kdevdefinesandincludesmanager_SRCS
    kcm_widget/batchedit.ui
    kcm_widget/includeswidget.ui
    kcm_widget/defineswidget.ui
    kcm_widget/projectpathswidget.ui
    kcm_widget/parserwidget.ui
)
kconfig_add_kcfg_files( kdevdefinesandincludesmanager_SRCS kcm_widget/customdefinesandincludes.kcfgc)

kdevplatform_add_plugin(KDevDefinesAndIncludesManager SOURCES ${kdevdefinesandincludesmanager_SRCS})
target_link_libraries(KDevDefinesAndIncludesManager
    PRIVATE
        kdevnoprojectincludesanddefines
        kdevcompilerprovider
        KDev::Project
        KDev::Util
        KDev::Language
        KF6::KIOWidgets
)

option(BUILD_kdev_includepathsconverter "Build utility to modify include paths of a project from command line." ON)
if(BUILD_kdev_includepathsconverter)
    add_executable(kdev_includepathsconverter includepathsconverter.cpp)
    ecm_mark_nongui_executable(kdev_includepathsconverter)
    target_link_libraries(kdev_includepathsconverter
        PRIVATE
            kdevcompilerprovider
            KDev::Project
    )
    install(TARGETS kdev_includepathsconverter ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )
endif()

install(FILES
    idefinesandincludesmanager.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevelop/custom-definesandincludes COMPONENT Devel
)

add_library(kdevdefinesandincludesmanager_interface INTERFACE)
add_library(KDev::DefinesAndIncludesManager ALIAS kdevdefinesandincludesmanager_interface)
target_include_directories(kdevdefinesandincludesmanager_interface INTERFACE
     "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>"
     "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/kdevelop>"
)
set_target_properties(kdevdefinesandincludesmanager_interface PROPERTIES
    EXPORT_NAME DefinesAndIncludesManager
)
install(TARGETS kdevdefinesandincludesmanager_interface EXPORT KDevelopTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
