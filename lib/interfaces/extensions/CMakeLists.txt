SUBDIRS(dcop)

SET(kdevextensions_SRCS kdevappfrontend.cpp kdevmakefrontend.cpp)

KDE_AUTOMOC(${kdevextensions_SRCS})

ADD_LIBRARY(kdevextensions STATIC ${kdevextensions_SRCS})