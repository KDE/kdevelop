add_definitions(-DTRANSLATION_DOMAIN=\"kdevoutlineview\")
if(UNIX)
  add_compile_options(-pedantic -Wno-gnu-zero-variadic-macro-arguments)
endif()

set(kdevoutlineview_SRCS
    outlineviewplugin.cpp
    outlinenode.cpp
    outlinemodel.cpp
    outlineproxymodel.cpp
    outlinewidget.cpp
)

kdevplatform_add_plugin(kdevoutlineview JSON kdevoutlineview.json SOURCES ${kdevoutlineview_SRCS})
target_link_libraries(kdevoutlineview KDev::Interfaces KDev::Language KDev::Project KDev::Util Qt5::Widgets KF5::CoreAddons KF5::I18n)

# install(FILES kdevoutlineview.rc DESTINATION ${DATA_INSTALL_DIR}/kdevoutlineview)
