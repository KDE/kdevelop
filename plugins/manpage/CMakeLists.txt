add_definitions(-DTRANSLATION_DOMAIN=\"kdevmanpage\")

declare_qt_logging_category(kdevmanpage_LOG_SRCS
    TYPE PLUGIN
    IDENTIFIER MANPAGE
    CATEGORY_BASENAME "manpage"
)
set(kdevmanpage_SRCS
    manpagedocumentation.cpp
    manpageplugin.cpp
    manpagemodel.cpp
    manpagedocumentationwidget.cpp
    ${kdevmanpage_LOG_SRCS}
)

kdevplatform_add_plugin(KDevManPage SOURCES ${kdevmanpage_SRCS})
target_link_libraries(KDevManPage KF6::TextEditor KDev::Language
KDev::Documentation KDev::Interfaces KF6::KCMUtils)

install(FILES
    manpagedocumentation.css
    DESTINATION ${KDE_INSTALL_DATADIR}/kdevmanpage
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
