project(krosssupport)
add_subdirectory(xmltokross)

get_directory_property(include_DIRS INCLUDE_DIRECTORIES)

get_directory_property(inclist INCLUDE_DIRECTORIES)
foreach(inc ${inclist})
    set(incs ${incs}:${inc})
endforeach(inc)

add_custom_target(krossWrappers)
macro(kdev_create_kross_iface wantImpl interface includes)
    get_filename_component(weFile ${interface} NAME_WE)
    set(weFile kross${weFile})
    set(outPath ${KDevPlatform_SOURCE_DIR}/shell/kross/wrappers/)
    get_filename_component(ifacePath interface PATH)
    
    if(${wantImpl} STREQUAL "TRUE")
#             message(STATUS "${weFile} has impl")
        set(theImpl "-impl")
    else()
        set(theImpl)
    endif()
    
    add_custom_target(${weFile}
	    		    rm -rf ~/.kdevduchain && ${CMAKE_CURRENT_BINARY_DIR}/xmltokross/duchaintokross ${theImpl} ${interface}
                            -I${ifacePath}:${incs} -F${weFile} -i${includes} -D${outPath}
                            -o${outPath}/${weFile}.cpp
                    #    DEPENDS ${interface} xmltokross/duchaintokross
                        )
    add_dependencies(krossWrappers ${weFile})
endmacro(kdev_create_kross_iface)

kdev_create_kross_iface(FALSE ${KDevPlatform_SOURCE_DIR}/interfaces/idocument.h interfaces/idocument.h)
kdev_create_kross_iface(FALSE ${KDevPlatform_SOURCE_DIR}/interfaces/iuicontroller.h interfaces/iuicontroller.h)
kdev_create_kross_iface(FALSE ${KDevPlatform_SOURCE_DIR}/interfaces/context.h interfaces/context.h)
kdev_create_kross_iface(FALSE ${KDevPlatform_SOURCE_DIR}/interfaces/contextmenuextension.h interfaces/contextmenuextension.h)
kdev_create_kross_iface(TRUE  ${KDevPlatform_SOURCE_DIR}/project/projectmodel.h project/projectmodel.h)
kdev_create_kross_iface(FALSE ${KDevPlatform_SOURCE_DIR}/vcs/vcsrevision.h vcs/vcsrevision.h)
kdev_create_kross_iface(FALSE ${KDevPlatform_SOURCE_DIR}/vcs/vcslocation.h vcs/vcslocation.h)
kdev_create_kross_iface(FALSE ${KDevPlatform_SOURCE_DIR}/interfaces/ilanguage.h interfaces/ilanguage.h)
kdev_create_kross_iface(FALSE ${KDevPlatform_SOURCE_DIR}/language/duchain/topducontext.h
    language/duchain/topducontext.h:language/duchain/parsingenvironment.h:language/interfaces/iproblem.h:language/editor/simplecursor.h)
kdev_create_kross_iface(FALSE ${KDevPlatform_SOURCE_DIR}/language/duchain/ducontext.h
    language/duchain/ducontext.h:language/duchain/topducontext.h)
kdev_create_kross_iface(FALSE ${KDevPlatform_SOURCE_DIR}/language/duchain/declaration.h
    language/duchain/declaration.h:language/duchain/declarationid.h:language/duchain/types/indexedtype.h:language/duchain/topducontext.h)
kdev_create_kross_iface(FALSE ${KDevPlatform_SOURCE_DIR}/language/interfaces/iproblem.h language/interfaces/iproblem.h:interfaces/iassistant.h)
kdev_create_kross_iface(FALSE ${KDevPlatform_SOURCE_DIR}/language/duchain/use.h language/duchain/use.h)
kdev_create_kross_iface(FALSE ${KDevPlatform_SOURCE_DIR}/language/duchain/identifier.h language/duchain/identifier.h:language/duchain/indexedstring.h)
kdev_create_kross_iface(FALSE ${KDevPlatform_SOURCE_DIR}/language/duchain/duchainlock.h language/duchain/duchainlock.h)
kdev_create_kross_iface(FALSE ${KDevPlatform_SOURCE_DIR}/project/interfaces/ibuildsystemmanager.h project/interfaces/ibuildsystemmanager.h)
kdev_create_kross_iface(FALSE ${KDevPlatform_SOURCE_DIR}/project/interfaces/iprojectfilemanager.h project/interfaces/iprojectfilemanager.h)
kdev_create_kross_iface(FALSE ${KDevPlatform_SOURCE_DIR}/project/interfaces/iprojectbuilder.h project/interfaces/iprojectbuilder.h)
kdev_create_kross_iface(FALSE ${KDevPlatform_SOURCE_DIR}/language/interfaces/editorcontext.h language/interfaces/editorcontext.h)
kdev_create_kross_iface(FALSE ${QT_QTGUI_INCLUDE_DIR}/qtoolbar.h QToolBar)
        
