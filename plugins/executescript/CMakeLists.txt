project(executescript)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9515 )

include_directories(
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

########### next target ###############

set(kdevexecutescript_PART_UIS
    scriptappconfig.ui
)

set(kdevexecutescript_PART_SRCS
    executescriptplugin.cpp
    scriptappconfig.cpp
    scriptappjob.cpp
)

qt5_wrap_ui( kdevexecutescript_PART_SRCS ${kdevexecutescript_PART_UIS} )
add_library(kdevexecutescript MODULE ${kdevexecutescript_PART_SRCS})
target_link_libraries(kdevexecutescript 
    ${KDEVPLATFORM_INTERFACES_LIBRARIES} 
    ${KDEVPLATFORM_UTIL_LIBRARIES} 
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_OUTPUTVIEW_LIBRARIES})

install(TARGETS kdevexecutescript DESTINATION ${PLUGIN_INSTALL_DIR}/kdevplatform/${KDEV_PLUGIN_VERSION})


########### install files ###############
install(FILES iexecutescriptplugin.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/executescript COMPONENT Devel)
configure_file(kdevexecutescript.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevexecutescript.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevexecutescript.desktop DESTINATION ${SERVICES_INSTALL_DIR})
