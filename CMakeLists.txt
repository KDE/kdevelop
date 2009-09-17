
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9515 )

########### next target ###############

set(kdevexecutescript_PART_UIS
    scriptappconfig.ui
)

set(kdevexecutescript_PART_SRCS
    executescriptplugin.cpp
    scriptappconfig.cpp
    scriptappjob.cpp
)

kde4_add_ui_files( kdevexecutescript_PART_SRCS ${kdevexecutescript_PART_UIS} )
kde4_add_plugin(kdevexecutescript ${kdevexecutescript_PART_SRCS}  )
target_link_libraries(kdevexecutescript 
    ${KDE4_KDEUI_LIBS} 
    kdevplatforminterfaces 
    kdevplatformutil 
    kdevplatformproject 
    kdevplatformoutputview)

install(TARGETS kdevexecutescript DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############
install(FILES kdevexecutescript.desktop DESTINATION ${SERVICES_INSTALL_DIR})
