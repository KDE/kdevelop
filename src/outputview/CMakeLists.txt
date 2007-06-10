include_directories(
    ${CMAKE_SOURCE_DIR}/src/interfaces
    ${CMAKE_SOURCE_DIR}/src/util
    ${CMAKE_CURRENT_SOURCE_DIR}
    )


 # interfaces

set( outputviewinterfaces_LIB_SRCS
    outputmodel.cpp
)
kde4_automoc( ${outputviewinterfaces_LIB_SRCS} )
kde4_add_library( kdevplatformoutputview SHARED ${outputviewinterfaces_LIB_SRCS} )
target_link_libraries(kdevplatformoutputview ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} )
set_target_properties(kdevplatformoutputview PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevplatformoutputview DESTINATION ${LIB_INSTALL_DIR} )

 ########### install files ###############
 install( FILES
          ioutputview.h
          outputmodel.h
          outputviewexport.h
          DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/outputview )
