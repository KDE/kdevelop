add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9508 )

########### next target ###############

set(kdevplatformutil_LIB_SRCS
    richtextpushbutton.cpp
    richtexttoolbutton.cpp
    kdevstringhandler.cpp
    focusedtreeview.cpp
    processlinemaker.cpp
    commandexecutor.cpp
    environmentconfigurebutton.cpp
    environmentselectionwidget.cpp
    environmentgrouplist.cpp
    activetooltip.cpp
    executecompositejob.cpp
)

set (kdevplatformutil_LIB_UI
    runoptions.ui
)

add_subdirectory(duchainify)

add_subdirectory(tests)

kde4_add_ui_files(kdevplatformutil_LIB_SRCS ${kdevplatformutil_LIB_US})
kde4_add_library(kdevplatformutil SHARED ${kdevplatformutil_LIB_SRCS})
target_link_libraries(kdevplatformutil 
        ${KDE4_KDEUI_LIBS} 
        ${KDE4_KCMUTILS_LIBRARY} 
        kdevplatforminterfaces
    )
# Might want to add kdevplatform* when they're exported targets
target_link_libraries(kdevplatformutil LINK_INTERFACE_LIBRARIES 
        ${KDE4_KDEUI_LIBS})
set_target_properties(kdevplatformutil PROPERTIES VERSION ${KDEVPLATFORM_LIB_VERSION} SOVERSION ${KDEVPLATFORM_LIB_SOVERSION})
install(TARGETS kdevplatformutil EXPORT KDevPlatformTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES
    richtextpushbutton.h
    richtexttoolbutton.h
    kdevstringhandler.h
    ksharedobject.h
    focusedtreeview.h
    activetooltip.h
    processlinemaker.h
    commandexecutor.h
    utilexport.h
    environmentconfigurebutton.h
    environmentselectionwidget.h
    environmentgrouplist.h
    pushvalue.h
    kdevvarlengtharray.h
    embeddedfreetree.h
    executecompositejob.h
    convenientfreelist.h
    spinlock.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/util COMPONENT Devel)

install( FILES
    google/dense_hash_map
    google/dense_hash_set
    google/sparse_hash_map
    google/sparse_hash_set
    google/sparsetable
    google/type_traits.h
    google/hash_fun.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/util/google COMPONENT Devel)

install( FILES
    google/sparsehash/densehashtable.h
    google/sparsehash/sparseconfig.h
    google/sparsehash/sparseconfig_windows.h
    google/sparsehash/sparsehashtable.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/util/google/sparsehash COMPONENT Devel)
