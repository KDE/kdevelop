add_library(kdev_filters MODULE kdevfilters.cpp)
ktexttemplate_adjust_plugin_name(kdev_filters)
target_link_libraries(kdev_filters
    KDev::Language
    KDev::Serialization
    KF6::TextTemplate
)

install(TARGETS kdev_filters DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/ktexttemplate)
