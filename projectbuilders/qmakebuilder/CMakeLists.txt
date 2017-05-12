add_definitions(-DTRANSLATION_DOMAIN=\"kdevqmakebuilder\")

set(qmakebuilder_SRCS
    qmakebuilder.cpp
    qmakebuilderpreferences.cpp
    qmakejob.cpp
)
ecm_qt_declare_logging_category(qmakebuilder_SRCS
    HEADER debug.h
    IDENTIFIER KDEV_QMAKEBUILDER
    CATEGORY_NAME "kdevelop.projectbuilders.qmakebuilder"
)
ki18n_wrap_ui(qmakebuilder_SRCS qmakeconfig.ui)
kconfig_add_kcfg_files(qmakebuilder_SRCS qmakebuilderconfig.kcfgc)
kdevplatform_add_plugin(kdevqmakebuilder JSON kdevqmakebuilder.json SOURCES ${qmakebuilder_SRCS})
target_link_libraries(kdevqmakebuilder
    KDev::Interfaces
    KDev::OutputView
    KDev::Util
    KDev::Project
    KDev::IMakeBuilder
    kdevqmakecommon
)

install( FILES iqmakebuilder.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevelop/qmake COMPONENT Devel)
install( FILES data.kdev4 DESTINATION ${KDE_INSTALL_DATADIR}/kdevqmakebuilder )
