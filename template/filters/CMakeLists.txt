add_library(kdev_filters MODULE kdevfilters.cpp)
grantlee_adjust_plugin_name(kdev_filters)
target_link_libraries(kdev_filters
    KDev::Language
    KDev::Serialization
    Grantlee5::Templates
)

install(TARGETS kdev_filters DESTINATION ${PLUGIN_INSTALL_DIR}/grantlee/${Grantlee_VERSION_MAJOR}.${Grantlee_VERSION_MINOR})
