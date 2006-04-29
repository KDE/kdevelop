kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})


########### next target ###############

set(kdevinterfaces_LIB_SRCS
    kdevapi.cpp
    kdevlanguagesupport.cpp
    kdevplugininfo.cpp
    kdevappfrontend.cpp
    kdevmainwindow.cpp
    kdevprojectbuilder.cpp
    kdevcatalog.cpp
    kdevmakefrontend.cpp
    kdevproject.cpp
    kdevcodemodel.cpp
    kdevdocumentcontroller.cpp
    kdevprojecteditor.cpp
    kdevcoderepository.cpp
    kdevdocumentcontrolleriface.cpp
    kdevprojectiface.cpp
    kdevcore.cpp
    kdevpcsimporter.cpp
    kdevprojectimporter.cpp
    kdevcoreiface.cpp
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
    kdevreadwritepart.cpp
    kdevdocument.cpp
)

kde4_automoc(${kdevinterfaces_LIB_SRCS})

set(kdevinterfaces_DCOP_SKEL_SRCS
    kdevdocumentcontrolleriface.h
    kdevcoreiface.h
    kdevprojectiface.h
)

kde4_add_dcop_skels(kdevinterfaces_LIB_SRCS ${kdevinterfaces_DCOP_SKEL_SRCS})

kde4_add_library(kdevinterfaces SHARED ${kdevinterfaces_LIB_SRCS})
target_link_libraries(kdevinterfaces ${KDE4_KDECORE_LIBS} kdeui kio kparts ktexteditor ${BERKELEY_DB_LIBRARIES})
set_target_properties(kdevinterfaces PROPERTIES VERSION 4.0.0 SOVERSION 4)
install_targets(/lib kdevinterfaces)


########### install files ###############

install_files(/include/kdevelop/interfaces FILES 
    kdevdocument.h
    kdevdocumentcontroller.h
    kdevapi.h
    kdevdocumentcontrolleriface.h
    kdevappfrontend.h
    kdevpcsimporter.h
    kdevcatalog.h
    kdevplugincontroller.h
    kdevcodemodel.h
    kdevplugin.h
    kdevcoderepository.h
    kdevplugininfo.h
    kdevcore.h
    kdevprojectbuilder.h
    kdevcoreiface.h
    kdevprojecteditor.h
    kdevcreatefile.h
    kdevproject.h
    kdevdifffrontend.h
    domutil.h
    kdevgenericfactory.h
    kdevprojectimporter.h
    kdevlanguagesupport.h
    kdevprojectmodel.h
    kdevmainwindow.h
    tag.h
    kdevmakefrontend.h
    kdevitemmodel.h
    filetemplate.h
    kdevcodedelegate.h
    kdevcodeproxy.h
    kdevreadwritepart.h
)

install_files(${SERVICETYPES_INSTALL_DIR} FILES 
    kdevelopappfrontend.desktop
    kdeveloppcsimporter.desktop
    kdevelopcreatefile.desktop
    kdevelopplugin.desktop
    kdevelopdifffrontend.desktop
    kdevelopprojectbuilder.desktop
    kdeveloplanguagesupport.desktop
    kdevelopproject.desktop
    kdevelopmakefrontend.desktop
    kdevelopprojectimporter.desktop
    kdevelop_part.desktop
    kdevelop_rwpart.desktop
)


kde4_footer()
