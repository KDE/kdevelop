include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )


 # interfaces

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9507 )
set( outputviewinterfaces_LIB_SRCS
    outputmodel.cpp
    ioutputviewmodel.cpp
    outputjob.cpp
)
kde4_add_library( kdevplatformoutputview SHARED ${outputviewinterfaces_LIB_SRCS} )
target_link_libraries(kdevplatformoutputview ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} kdevplatforminterfaces  )
set_target_properties(kdevplatformoutputview PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kdevplatformoutputview ${INSTALL_TARGETS_DEFAULT_ARGS} )

 ########### install files ###############
 install( FILES
          ioutputview.h
          outputmodel.h
          ioutputviewmodel.h
          outputviewexport.h
          outputjob.h
          DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/outputview )
