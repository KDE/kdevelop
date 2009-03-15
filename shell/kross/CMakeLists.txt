project(krosssupport)
add_subdirectory(xmltokross)

set(KDEVCPPDUCHAIN "" CACHE STRING "kdevcpp location")
if(KDEVCPPDUCHAIN)
    get_directory_property(include_DIRS INCLUDE_DIRECTORIES)
    
    get_directory_property(inclist INCLUDE_DIRECTORIES)
    foreach(inc ${inclist})
        set(incs ${incs}:${inc})
    endforeach(inc)
    
    add_custom_target(krossWrappers)
    macro(kdev_create_kross_iface wantImpl interface includes)
        get_filename_component(weFile ${interface} NAME_WE)
        set(weFile kross${weFile})
        set(outPath ${CMAKE_SOURCE_DIR}/kross/wrappers/)
        get_filename_component(ifacePath interface PATH)
        
        if(${wantImpl} STREQUAL "TRUE")
#             message(STATUS "${weFile} has impl")
            set(theImpl "-impl")
        else(${wantImpl} STREQUAL "TRUE")
            set(theImpl)
        endif(${wantImpl} STREQUAL "TRUE")
        
        add_custom_target(${weFile}
                            ${CMAKE_CURRENT_BINARY_DIR}/xmltokross/duchaintokross ${theImpl} ${interface}
                                -I${ifacePath}:${incs} -F${weFile} -i${includes} -D${outPath}
                                -o${CMAKE_SOURCE_DIR}/kross/wrappers/${weFile}.cpp
                        #    DEPENDS ${interface} xmltokross/duchaintokross
                         )
        add_dependencies(krossWrappers ${weFile})
    endmacro(kdev_create_kross_iface)

    kdev_create_kross_iface(FALSE ${CMAKE_SOURCE_DIR}/interfaces/idocument.h interfaces/idocument.h)
    kdev_create_kross_iface(FALSE ${CMAKE_SOURCE_DIR}/interfaces/iuicontroller.h interfaces/iuicontroller.h)
    kdev_create_kross_iface(FALSE ${CMAKE_SOURCE_DIR}/interfaces/context.h interfaces/context.h)
    kdev_create_kross_iface(FALSE ${CMAKE_SOURCE_DIR}/interfaces/contextmenuextension.h interfaces/contextmenuextension.h)
    kdev_create_kross_iface(TRUE  ${CMAKE_SOURCE_DIR}/project/projectmodel.h project/projectmodel.h)
    kdev_create_kross_iface(FALSE ${CMAKE_SOURCE_DIR}/vcs/vcsrevision.h vcs/vcsrevision.h)
    kdev_create_kross_iface(FALSE ${CMAKE_SOURCE_DIR}/vcs/vcslocation.h vcs/vcslocation.h)
    kdev_create_kross_iface(FALSE ${CMAKE_SOURCE_DIR}/interfaces/ilanguage.h interfaces/ilanguage.h)
    kdev_create_kross_iface(FALSE ${CMAKE_SOURCE_DIR}/language/duchain/topducontext.h
        language/duchain/topducontext.h:language/duchain/parsingenvironment.h:language/interfaces/iproblem.h:language/editor/simplecursor.h)
    kdev_create_kross_iface(FALSE ${CMAKE_SOURCE_DIR}/language/duchain/ducontext.h
        language/duchain/ducontext.h:language/duchain/topducontext.h)
    kdev_create_kross_iface(FALSE ${CMAKE_SOURCE_DIR}/language/duchain/declaration.h
        language/duchain/declaration.h:language/duchain/declarationid.h:language/duchain/types/indexedtype.h:language/duchain/topducontext.h)
    kdev_create_kross_iface(FALSE ${CMAKE_SOURCE_DIR}/language/interfaces/iproblem.h language/interfaces/iproblem.h)
    kdev_create_kross_iface(FALSE ${CMAKE_SOURCE_DIR}/language/duchain/use.h language/duchain/use.h)
    kdev_create_kross_iface(FALSE ${CMAKE_SOURCE_DIR}/language/duchain/identifier.h language/duchain/identifier.h:language/duchain/indexedstring.h)
    kdev_create_kross_iface(FALSE ${CMAKE_SOURCE_DIR}/language/duchain/duchainlock.h language/duchain/duchainlock.h)
    kdev_create_kross_iface(FALSE ${CMAKE_SOURCE_DIR}/project/interfaces/ibuildsystemmanager.h project/interfaces/ibuildsystemmanager.h)
    kdev_create_kross_iface(FALSE ${CMAKE_SOURCE_DIR}/project/interfaces/iprojectfilemanager.h project/interfaces/iprojectfilemanager.h)
    kdev_create_kross_iface(FALSE ${CMAKE_SOURCE_DIR}/project/interfaces/iprojectbuilder.h project/interfaces/iprojectbuilder.h)
    kdev_create_kross_iface(FALSE ${CMAKE_SOURCE_DIR}/interfaces/irun.h interfaces/irun.h)
    kdev_create_kross_iface(FALSE ${CMAKE_SOURCE_DIR}/language/interfaces/editorcontext.h language/interfaces/editorcontext.h)
    kdev_create_kross_iface(FALSE ${QT_QTGUI_INCLUDE_DIR}/qtoolbar.h QToolBar)
        
else(KDEVCPPDUCHAIN)
    message(STATUS "KDEVCPPDUCHAIN pointing to cppduchain directory in kdevelop not in the cache. Re-Generating kross-wrappers not supported, will use the pre-generated code.")
endif(KDEVCPPDUCHAIN)

set(kdevkrossplugin_SRCS
    krossplugin.cpp
    krossbuildsystemmanager.cpp
    krossdistributedversioncontrol.cpp
    krosstoolviewfactory.cpp
    krossvcsjob.cpp
    wrappers/krossiuicontroller.cpp
    wrappers/krossidocument.cpp
    wrappers/krosscontext.cpp
    wrappers/krosscontextmenuextension.cpp
    wrappers/krossvcsrevision.cpp
    wrappers/krossvcslocation.cpp
    wrappers/krosstopducontext.cpp
    wrappers/krossducontext.cpp
    wrappers/krossprojectmodel.cpp
    wrappers/krossilanguage.cpp
    wrappers/krossdeclaration.cpp
    wrappers/krossuse.cpp
    wrappers/krossidentifier.cpp
    wrappers/krossduchainlock.cpp
    wrappers/krossiproblem.cpp
    wrappers/krossibuildsystemmanager.cpp
    wrappers/krossiprojectfilemanager.cpp
    wrappers/krossiprojectbuilder.cpp
    wrappers/krossirun.cpp
    wrappers/krosseditorcontext.cpp
    wrappers/krossqtoolbar.cpp
)

kde4_add_plugin(kdevkrossplugin ${kdevkrossplugin_SRCS})
target_link_libraries(kdevkrossplugin ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_KROSSCORE_LIBS}
    kdevplatforminterfaces kdevplatformproject kdevplatformvcs kdevplatformlanguage kdevplatformveritas ${KDE4_KROSSUI_LIBS})

install(TARGETS kdevkrossplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kdevkrossplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})

