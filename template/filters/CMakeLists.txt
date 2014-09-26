add_library(kdev_filters MODULE kdevfilters.cpp)
grantlee_adjust_plugin_name(kdev_filters)
target_link_libraries(kdev_filters
    KDev::Language
    KDev::Serialization
    Grantlee5::Templates
)

get_property(Grantlee_PLUGIN_DIR TARGET Grantlee5::defaulttags PROPERTY LOCATION)
get_filename_component(Grantlee_PLUGIN_DIR ${Grantlee_PLUGIN_DIR} PATH)

install(TARGETS kdev_filters DESTINATION ${Grantlee_PLUGIN_DIR})
