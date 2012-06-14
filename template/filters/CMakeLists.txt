set(kdevfilters_SRCS kdevfilters.cpp)

grantlee_add_plugin(grantlee_kdevfilters kdevfilters)

install(TARGETS grantlee_kdevfilters ${TEMPLATE_FILTERS_INSTALL_ARGS})
