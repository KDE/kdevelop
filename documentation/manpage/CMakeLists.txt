add_definitions(-DTRANSLATION_DOMAIN=\"kdevmanpage\")
set(kdevmanpage_SRCS
    manpagedocumentation.cpp
    manpageplugin.cpp
    manpagemodel.cpp
    manpagedocumentationwidget.cpp
)

kdevplatform_add_plugin(kdevmanpage JSON kdevmanpage.json SOURCES ${kdevmanpage_SRCS})
target_link_libraries(kdevmanpage KF5::TextEditor KDev::Language
KDev::Documentation KDev::Interfaces KF5::KCMUtils Qt5::WebKitWidgets)

install(FILES
    manpagedocumentation.css
    DESTINATION ${DATA_INSTALL_DIR}/kdevmanpage
)

add_subdirectory(tests)
