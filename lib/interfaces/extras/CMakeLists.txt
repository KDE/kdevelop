SET(kdevextras_SRCS kdevcompileroptions.cpp kdevvcsintegrator.cpp)

KDE_AUTOMOC(${kdevextras_SRCS})

KDE_ADD_MOC_FILES(kdevextras_SRCS kdevcompileroptions.h)

ADD_LIBRARY(kdevextras SHARED  kdevcompileroptions.cpp kdevvcsintegrator.cpp)

