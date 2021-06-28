add_definitions(-DTRANSLATION_DOMAIN=\"kdevmanpage\")
set(kdevmanpage_SRCS
    manpagedocumentation.cpp
    manpageplugin.cpp
    manpagemodel.cpp
    manpagedocumentationwidget.cpp
)

kdevplatform_add_plugin(kdevmanpage SOURCES ${kdevmanpage_SRCS})
target_link_libraries(kdevmanpage KF5::TextEditor KDev::Language
KDev::Documentation KDev::Interfaces KF5::KCMUtils)

install(FILES
    manpagedocumentation.css
    DESTINATION ${KDE_INSTALL_DATADIR}/kdevmanpage
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
