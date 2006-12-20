
add_subdirectory(util)
add_subdirectory(widgets)
add_subdirectory(debugger)
add_subdirectory(projectmanager)
add_subdirectory(settings)

include_directories(
 ${CMAKE_SOURCE_DIR}/lib/util
 ${CMAKE_SOURCE_DIR}/lib/widgets
 ${CMAKE_SOURCE_DIR}/lib/editor
 ${CMAKE_SOURCE_DIR}/lib/backgroundparser
 ${CMAKE_SOURCE_DIR}/lib/interfaces
 ${CMAKE_SOURCE_DIR}/lib/project
 )
if (BERKELEY_DB_FOUND)
    include_directories(${BERKELEY_DB_INCLUDE_DIR})
endif (BERKELEY_DB_FOUND)
if (TDB_FOUND)
    include_directories(${TDB_INCLUDE_DIR})
endif (TDB_FOUND)


########### next target ###############

set(kdevplatform_LIB_SRCS
    kdevlanguagesupport.cpp
    kdevmainwindow.cpp
    kdevcodemodel.cpp
    kdevdocumentcontroller.cpp
    kdevcoderepository.cpp
    kdevcore.cpp
    kdevpcsimporter.cpp
    kdevplugincontroller.cpp
    kdevplugin.cpp
    domutil.cpp
    kdevitemmodel.cpp
    filetemplate.cpp
    kdevcodedelegate.cpp
    kdevcodeproxy.cpp
    kdevcodeaggregate.cpp
    kdevdocument.cpp
    kdevfilemanager.cpp
    kdevbuildmanager.cpp
    kdevprofile.cpp
    kdevprofileengine.cpp
    kdevcmodule.cpp
    kdevconfigskeleton.cpp
    kdevenvironment.cpp
    kdevconfig.cpp
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
    tag.cpp
    editor/kdeveditorintegrator.cpp
    editor/kdevdocumentrangeobject.cpp
    editor/kdevdocumentcursorobject.cpp
    editor/kdevdocumentcursor.cpp
    editor/kdevdocumentrange.cpp
    backgroundparser/kdevparserdependencypolicy.cpp
    interfaces/kdevapplicationinterface.cpp
    interfaces/kdevdiffinterface.cpp
    interfaces/kdevmakeinterface.cpp
    project/kdevprojectbuilder.cpp
    project/kdevproject.cpp
    project/kdevprojectmodel.cpp
    project/kdevprojectcontroller.cpp
    project/importprojectjob.cpp
)

if (BERKELEY_DB_FOUND)
    set(kdevplatform_LIB_SRCS
        ${kdevplatform_LIB_SRCS}
        kdevbdbcatalogbackend.cpp
       )
endif (BERKELEY_DB_FOUND)

if (TDB_FOUND)
    set(kdevplatform_LIB_SRCS
        ${kdevplatform_LIB_SRCS}
        kdevtdbcatalogbackend.cpp
       )
endif (TDB_FOUND)

set(kdevplatform_UI
    mimewarningdialog.ui
)

kde4_automoc(${kdevplatform_LIB_SRCS})

kde4_add_ui_files(kdevplatform_LIB_SRCS ${kdevplatform_UI})
kde4_add_library(kdevplatform SHARED ${kdevplatform_LIB_SRCS})
target_link_libraries(kdevplatform ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KNOTIFYCONFIG_LIBS} ktexteditor ${KDE4_THREADWEAVER_LIBRARIES} ${BERKELEY_DB_LIBRARIES} ${TDB_LIBRARIES} )
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
    kdevcreatefile.h
    domutil.h
    kdevlanguagesupport.h
    kdevmainwindow.h
    kdevitemmodel.h
    filetemplate.h
    kdevcodedelegate.h
    kdevcodeproxy.h
    kdevfilemanager.h
    kdevbuildmanager.h
    kdevprofile.h
    kdevprofileengine.h
    kdevcmodule.h
    kdevconfigskeleton.h
    kdevenvironment.h
    kdevconfig.h
    kdevsharedptr.h
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
    project/kdevprojectbuilder.h
    project/kdevproject.h
    project/kdevprojectmodel.h
    project/kdevprojectcontroller.h
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

