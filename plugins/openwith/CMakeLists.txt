add_definitions(-DTRANSLATION_DOMAIN=\"kdevopenwith\")
set(kdevopenwith_PART_SRCS
    openwithplugin.cpp
)

kdevplatform_add_plugin(kdevopenwith SOURCES ${kdevopenwith_PART_SRCS})
target_link_libraries(kdevopenwith KDev::Interfaces KDev::Project KDev::Util KF6::Service KF6::KIOWidgets)

########### install files ###############

install( FILES iopenwith.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kdevplatform/openwith COMPONENT Devel)
