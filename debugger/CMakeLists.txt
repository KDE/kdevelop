#TODO: add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9507 )


########### next target ###############

set( kdevplatformdebugger_LIB_SRCS
    interfaces/idebugsession.cpp
    interfaces/stackmodel.cpp
    util/treeitem.cpp
    util/treemodel.cpp
    util/treeview.cpp
)
kde4_add_library(kdevplatformdebugger SHARED ${kdevplatformdebugger_LIB_SRCS})
target_link_libraries(kdevplatformdebugger 
        ${QT_QTCORE_LIBRARY} 
        ${KDE4_KDEUI_LIBS}
    )
set_target_properties(kdevplatformdebugger PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})
install(TARGETS kdevplatformdebugger EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install(FILES
    debuggerexport.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/debugger COMPONENT Devel
)
install(FILES
    interfaces/idebugsession.h
    interfaces/stackmodel.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/debugger/interfaces COMPONENT Devel
)
install(FILES
    util/treemodel.h
    util/treeitem.h
    util/treeview.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/debugger/util COMPONENT Devel
)
