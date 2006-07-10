
include_directories( ${CMAKE_SOURCE_DIR}/lib )


########### next target ###############

set(kdevinterfaces_LIB_SRCS
    kdevapi.cpp
    kdevlanguagesupport.cpp
    kdevappfrontend.cpp
    kdevmainwindow.cpp
    kdevprojectbuilder.cpp
    kdevcatalog.cpp
    kdevmakefrontend.cpp
    kdevproject.cpp
    kdevcodemodel.cpp
    kdevdocumentcontroller.cpp
    kdevcoderepository.cpp
    kdevcore.cpp
    kdevpcsimporter.cpp
    kdevplugincontroller.cpp
    kdevprojectmodel.cpp
    kdevdifffrontend.cpp
    kdevplugin.cpp
    tag.cpp
    domutil.cpp
    kdevitemmodel.cpp
    filetemplate.cpp
    kdevcodedelegate.cpp
    kdevcodeproxy.cpp
    kdevcodeaggregate.cpp
    kdevdocument.cpp
    kdevbuildableproject.cpp
    kdevfilemanager.cpp
    kdevbuildmanager.cpp
    kdevprofile.cpp
    kdevprofileengine.cpp
    kdevcmodule.cpp
    kdevconfigskeleton.cpp
)

kde4_automoc(${kdevinterfaces_LIB_SRCS})

kde4_add_library(kdevinterfaces SHARED ${kdevinterfaces_LIB_SRCS})
target_link_libraries(kdevinterfaces ${KDE4_KDECORE_LIBS} kdeui kio kparts ktexteditor ${BERKELEY_DB_LIBRARIES})
set_target_properties(kdevinterfaces PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevinterfaces DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install(FILES
    kdevdocument.h
    kdevdocumentcontroller.h
    kdevapi.h
    kdevappfrontend.h
    kdevpcsimporter.h
    kdevcatalog.h
    kdevplugincontroller.h
    kdevcodemodel.h
    kdevplugin.h
    kdevcoderepository.h
    kdevcore.h
    kdevprojectbuilder.h
    kdevcreatefile.h
    kdevproject.h
    kdevdifffrontend.h
    domutil.h
    kdevlanguagesupport.h
    kdevprojectmodel.h
    kdevmainwindow.h
    tag.h
    kdevmakefrontend.h
    kdevitemmodel.h
    filetemplate.h
    kdevcodedelegate.h
    kdevcodeproxy.h
    kdevfilemanager.h
    kdevbuildableproject.h
    kdevbuildmanager.h
    kdevprofile.h
    kdevprofileengine.h
    kdevcmodule.h
    kdevconfigskeleton.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/interfaces 
)

install(FILES
    kdevelopappfrontend.desktop
    kdeveloppcsimporter.desktop
    kdevelopcreatefile.desktop
    kdevelopplugin.desktop
    kdevelopdifffrontend.desktop
    kdevelopprojectbuilder.desktop
    kdeveloplanguagesupport.desktop
    kdevelopproject.desktop
    kdevelopmakefrontend.desktop
    kdevelopprojectfilemanager.desktop
    kdevelop_part.desktop
    kdevelop_rwpart.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR} 
)

