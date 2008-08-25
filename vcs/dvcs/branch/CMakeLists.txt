project(branch)

find_package(KDE4 REQUIRED)

include_directories( ${KDE4_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/
    /workspace/gsoc/kdevplatform/
    /workspace/gsoc/kdevplatform/vcs
    /workspace/gsoc/kdevplatform/util
    /workspace/gsoc/kdevplatform/interfaces
    /workspace/gsoc/kdevplatform/project
    ${CMAKE_CURRENT_SOURCE_DIR}/../../interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/../../models
    ${CMAKE_CURRENT_SOURCE_DIR}/../ui/revhistory/
)


########### next target ###############

set(branch_PART_SRCS
    ../ui/revhistory/commitlogmodel.cpp
    ../ui/revhistory/commitView.cpp
    ../../../plugins/git/gitexecutor.cpp
    test.cpp
)

# set(branch_UIS
#     commitdialog.ui
# )

kde4_add_ui_files(branch_PART_SRCS ${branch_UIS})

kde4_add_executable(branch ${branch_PART_SRCS})

target_link_libraries(branch
    ${KDE4_KDEUI_LIBS}
    kdevplatformutil
    kdevplatforminterfaces
    kdevplatformvcs
    kdevplatformproject
)


