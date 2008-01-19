include_directories(
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/util
    ${CMAKE_CURRENT_SOURCE_DIR}
    )


 # interfaces

set( outputviewinterfaces_LIB_SRCS
    outputmodel.cpp
    ioutputviewmodel.cpp
)
kde4_add_library( kdevplatformoutputview SHARED ${outputviewinterfaces_LIB_SRCS} )
target_link_libraries(kdevplatformoutputview ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} )
set_target_properties(kdevplatformoutputview PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kdevplatformoutputview DESTINATION ${LIB_INSTALL_DIR} )

 ########### install files ###############
 install( FILES
          ioutputview.h
          outputmodel.h
          ioutputviewmodel.h
          outputviewexport.h
          DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/outputview )
