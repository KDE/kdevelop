add_definitions(-DTRANSLATION_DOMAIN=\"kdevopenwith\")
set(kdevopenwith_PART_SRCS
    openwithplugin.cpp
)

kdevplatform_add_plugin(kdevopenwith JSON kdevopenwith.json SOURCES ${kdevopenwith_PART_SRCS})
target_link_libraries(kdevopenwith KDev::Interfaces KDev::Project KDev::Util)

########### install files ###############

install( FILES iopenwith.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/openwith COMPONENT Devel)
