SET(kdevutils_SRCS 
domutil.cpp 
execcommand.cpp 
filetemplate.cpp
urlutil.cpp 
configwidgetproxy.cpp
rurl.cpp
kscriptactionmanager.cpp
settings.cpp
)

KDE_AUTOMOC(${kdevutils_SRCS})

ADD_LIBRARY(kdevutils SHARED ${kdevutils_SRCS})

