
SET(kdevdcopextensions_SRCS KDevAppFrontendIface.cpp KDevMakeFrontendIface.cpp )


SET(kdevdcopextensions_DCOP_SRCS KDevAppFrontendIface.h KDevMakeFrontendIface.h )

KDE_ADD_DCOP_SKELS(kdevdcopextensions_SRCS ${kdevdcopextensions_DCOP_SRCS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/..)

ADD_LIBRARY(kdevdcopextensions ${kdevdcopextensions_SRCS})