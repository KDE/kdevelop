set( makefileresolver_SRCS
        makefileresolver.cpp
   )

kde4_add_library(kdev4makefileresolver STATIC
        ${makefileresolver_SRCS}
    )

set_target_properties(kdev4makefileresolver PROPERTIES POSITION_INDEPENDENT_CODE ON)

option(BUILD_kdev_makefileresolver "Build the makefile resolver debugging tool" OFF)
if(BUILD_kdev_makefileresolver)
  add_executable(kdev_makefileresolver makefileresolver.cpp)
  set_target_properties( kdev_makefileresolver PROPERTIES COMPILE_FLAGS -DTEST )
  target_link_libraries( kdev_makefileresolver
  ${KDEVPLATFORM_INTERFACES_LIBRARIES}  ${KDEVPLATFORM_PROJECT_LIBRARIES}
  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
  ${KDEVPLATFORM_TESTS_LIBRARIES}
)
  install(TARGETS kdev_makefileresolver ${INSTALL_TARGETS_DEFAULT_ARGS} )
endif(BUILD_kdev_makefileresolver)

add_subdirectory(tests)