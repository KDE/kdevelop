add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9531 )


########### next target ###############

set( kdevplatformdebugger_LIB_SRCS
    interfaces/idebugsession.cpp
    interfaces/iframestackmodel.cpp
    interfaces/ibreakpointcontroller.cpp
    interfaces/ivariablecontroller.cpp
    util/treeitem.cpp
    util/treemodel.cpp
    util/treeview.cpp
    util/pathmappings.cpp
    breakpoint/breakpoint.cpp
    breakpoint/breakpointmodel.cpp
    breakpoint/breakpointwidget.cpp
    breakpoint/breakpointdetails.cpp
    variable/variablewidget.cpp
    variable/variablecollection.cpp
    variable/variabletooltip.cpp
    framestack/framestackmodel.cpp
    framestack/framestackwidget.cpp
)
add_library(kdevplatformdebugger SHARED ${kdevplatformdebugger_LIB_SRCS})
target_link_libraries(kdevplatformdebugger LINK_PUBLIC
    kdevplatforminterfaces
    kdevplatformutil
)
target_link_libraries(kdevplatformdebugger LINK_PRIVATE
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    KF5::TextEditor
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
    interfaces/ibreakpointcontroller.h
    interfaces/ivariablecontroller.h
    interfaces/iframestackmodel.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/debugger/interfaces COMPONENT Devel
)
install(FILES
    util/treemodel.h
    util/treeitem.h
    util/treeview.h
    util/pathmappings.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/debugger/util COMPONENT Devel
)
install(FILES
    breakpoint/breakpointwidget.h
    breakpoint/breakpointdetails.h
    breakpoint/breakpoint.h
    breakpoint/breakpointmodel.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/debugger/breakpoint COMPONENT Devel
)
install(FILES
    variable/variablecollection.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/debugger/variable COMPONENT Devel
)
install(FILES
    framestack/framestackmodel.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/debugger/framestack COMPONENT Devel
)
