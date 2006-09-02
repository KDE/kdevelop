
add_subdirectory(util)
add_subdirectory(widgets)
add_subdirectory(debugger)
add_subdirectory(projectmanager)
add_subdirectory(settings)

include_directories( ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets ${THREADWEAVER_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/lib/editor ${CMAKE_SOURCE_DIR}/lib/backgroundparser ${CMAKE_SOURCE_DIR}/lib/interfaces )

########### next target ###############

set(kdevplatform_LIB_SRCS
    kdevlanguagesupport.cpp
    kdevmainwindow.cpp
    kdevprojectbuilder.cpp
    kdevproject.cpp
    kdevcodemodel.cpp
    kdevdocumentcontroller.cpp
    kdevcoderepository.cpp
    kdevcore.cpp
    kdevpcsimporter.cpp
    kdevplugincontroller.cpp
    kdevprojectmodel.cpp
    kdevplugin.cpp
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
    kdevenvironment.cpp
    kdevconfig.cpp
    kdevprojectcontroller.cpp
    kdevpartcontroller.cpp
    kdevlanguagecontroller.cpp
    kdevstatusbar.cpp
    shellextension.cpp
    kdevcontext.cpp
    kdevast.cpp
    kdevbackgroundparser.cpp
    kdevparsejob.cpp
    kdevpersistenthash.cpp
    kdevcodehighlighting.cpp
    editor/kdeveditorintegrator.cpp
    editor/kdevdocumentrangeobject.cpp
    editor/kdevdocumentcursorobject.cpp
    editor/kdevdocumentcursor.cpp
    editor/kdevdocumentrange.cpp
    backgroundparser/kdevparserdependencypolicy.cpp
    interfaces/kdevapplicationinterface.cpp
    interfaces/kdevdiffinterface.cpp
    interfaces/kdevmakeinterface.cpp
)

set(kdevplatform_UI3
    mimewarningdialog.ui
)

kde4_automoc(${kdevplatform_LIB_SRCS})

kde4_add_ui3_files(kdevplatform_LIB_SRCS ${kdevplatform_UI3})
kde4_add_library(kdevplatform SHARED ${kdevplatform_LIB_SRCS})
target_link_libraries(kdevplatform ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_KPARTS_LIBS} ktexteditor ThreadWeaver ${BERKELEY_DB_LIBRARIES} )
set_target_properties(kdevplatform PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevplatform DESTINATION ${LIB_INSTALL_DIR} )

########### install files ###############

install(FILES
    kdevdocument.h
    kdevdocumentcontroller.h
    kdevpcsimporter.h
    kdevplugincontroller.h
    kdevcodemodel.h
    kdevplugin.h
    kdevcoderepository.h
    kdevcore.h
    kdevprojectbuilder.h
    kdevcreatefile.h
    kdevproject.h
    domutil.h
    kdevlanguagesupport.h
    kdevprojectmodel.h
    kdevmainwindow.h
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
    kdevenvironment.h
    kdevconfig.h
    kdevsharedptr.h
    kdevprojectcontroller.h
    kdevpartcontroller.h
    kdevlanguagecontroller.h
    kdevstatusbar.h
    shellextension.h
    kdevcontext.h
    kdevast.h
    kdevbackgroundparser.h
    kdevparsejob.h
    kdevpersistenthash.h
    kdevcodehighlighting.h
    editor/kdeveditorintegrator.h
    editor/kdevdocumentrangeobject.h
    editor/kdevdocumentcursorobject.h
    editor/kdevdocumentcursor.h
    editor/kdevdocumentrange.h
    interfaces/kdevapplicationinterface.h
    interfaces/kdevdiffinterface.h
    interfaces/kdevmakeinterface.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/
)

install(FILES
    kdeveloppcsimporter.desktop
    kdevelopcreatefile.desktop
    kdevelopplugin.desktop
    kdevelopprojectbuilder.desktop
    kdeveloplanguagesupport.desktop
    kdevelopproject.desktop
    kdevelopprojectfilemanager.desktop
    kdevelop_part.desktop
    kdevelop_rwpart.desktop
    interfaces/kdevapplicationinterface.desktop
    interfaces/kdevdiffinterface.desktop
    interfaces/kdevmakeinterface.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR}
)

install( FILES kdevexport.h  DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/)

