set(kdevfilters_SRCS kdevfilters.cpp)

grantlee_add_plugin(kdev_filters kdevfilters)
target_link_libraries(kdev_filters ${KDE4_KDECORE_LIBS} kdevplatformlanguage)

install(TARGETS kdev_filters DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/grantlee/${Grantlee_VERSION_MAJOR}.${Grantlee_VERSION_MINOR})
