add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9508 )

########### next target ###############

set(kdevplatformutil_LIB_SRCS
    processlinemaker.cpp
    commandexecutor.cpp
    environmentselectionwidget.cpp
    environmentgrouplist.cpp
    sourceformattermanager.cpp
    interfaces/isourceformatter.cpp
)

set (kdevplatformutil_LIB_UI
    runoptions.ui
)

kde4_add_ui_files(kdevplatformutil_LIB_SRCS ${kdevplatformutil_LIB_US})
kde4_add_library(kdevplatformutil SHARED ${kdevplatformutil_LIB_SRCS})
target_link_libraries(kdevplatformutil ${KDE4_KDEUI_LIBS} ${KDE4_KUTILS_LIBRARY} kdevplatforminterfaces kdevplatformoutputview kdevplatformlanguage kdevplatformproject)
set_target_properties(kdevplatformutil PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})
install(TARGETS kdevplatformutil ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES
    processlinemaker.h
    commandexecutor.h
    utilexport.h
    environmentselectionwidget.h
    environmentgrouplist.h
    pushvalue.h
    kdevvarlengtharray.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/util)
    
install( FILES
    interfaces/isourceformatter.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/util/interfaces)

install( FILES
    google/dense_hash_map
    google/dense_hash_set
    google/sparse_hash_map
    google/sparse_hash_set
    google/sparsetable
    google/type_traits.h
    google/hash_fun.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/util/google)

install( FILES
    google/sparsehash/densehashtable.h
    google/sparsehash/sparseconfig.h
    google/sparsehash/sparseconfig_windows.h
    google/sparsehash/sparsehashtable.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/util/google/sparsehash )
