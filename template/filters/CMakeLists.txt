set(kdevfilters_SRCS kdevfilters.cpp)

grantlee_add_plugin(kdev_filters kdevfilters)
target_link_libraries(kdev_filters ${KDE4_KDECORE_LIBS} kdevplatformlanguage)

install(TARGETS kdev_filters DESTINATION ${TEMPLATE_FILTERS_INSTALL_ARGS})
