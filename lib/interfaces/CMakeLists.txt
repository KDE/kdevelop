SUBDIRS(extensions external)

SET(kdevinterfaces_SOURCES 
kdevcore.cpp
kdevproject.cpp 
kdevlanguagesupport.cpp
kdevpartcontroller.cpp
kdevapi.cpp
kdevmainwindow.cpp
kdevplugin.cpp
kdevcoderepository.cpp 
codemodel.cpp
codemodel_treeparser.cpp
codemodel_utils.cpp 
kdevdesignerintegration.cpp
kdevplugincontroller.cpp
kdevplugininfo.cpp
KDevCoreIface.cpp 
KDevPartControllerIface.cpp
katedocumentmanagerinterface.cpp
kdevprojectiface.cpp
kdevdesignerintegrationiface.cpp 
)

SET(kdevinterfaces_DCOP_SRCS
KDevCoreIface.h
KDevPartControllerIface.h
katedocumentmanagerinterface.h
kdevprojectiface.h
kdevdesignerintegrationiface.h
)

KDE_AUTOMOC(${kdevinterfaces_SOURCES})

KDE_ADD_DCOP_SKELS(kdevinterfaces_SOURCES ${kdevinterfaces_DCOP_SRCS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/external/ )

ADD_LIBRARY(kdevinterfaces STATIC ${kdevinterfaces_SOURCES})
