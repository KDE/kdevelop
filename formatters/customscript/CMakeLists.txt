########### indent ###############
set(kdevcustomscript_PART_SRCS
    customscript_plugin.cpp
)

kdevplatform_add_plugin(kdevcustomscript JSON kdevcustomscript.json SOURCES ${kdevcustomscript_PART_SRCS})
target_link_libraries(kdevcustomscript
    KDev::Interfaces
    KDev::Util)
