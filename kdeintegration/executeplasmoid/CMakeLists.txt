project(executeplasmoid)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9515 )

include_directories(
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDE4_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

########### next target ###############

set(kdevexecuteplasmoid_PART_UIS
    plasmoidexecutionconfig.ui
)

set(kdevexecuteplasmoid_PART_SRCS
    executeplasmoidplugin.cpp
    plasmoidexecutionconfig.cpp
    plasmoidexecutionjob.cpp
)

kde4_add_ui_files( kdevexecuteplasmoid_PART_SRCS ${kdevexecuteplasmoid_PART_UIS} )
kde4_add_plugin(kdevexecuteplasmoid ${kdevexecuteplasmoid_PART_SRCS}  )
target_link_libraries(kdevexecuteplasmoid 
    ${KDE4_KDEUI_LIBS}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES} 
    ${KDEVPLATFORM_INTERFACES_LIBRARIES} 
    ${KDEVPLATFORM_UTIL_LIBRARIES} 
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES})

install(TARGETS kdevexecuteplasmoid DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############
configure_file(kdevexecuteplasmoid.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevexecuteplasmoid.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevexecuteplasmoid.desktop DESTINATION ${SERVICES_INSTALL_DIR})
