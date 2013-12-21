find_file( KDEV_IPLUGIN_H kdevplatform/interfaces/iplugin.h )
if(KDEV_IPLUGIN_H)
        file(STRINGS ${KDEV_IPLUGIN_H} KDEV_PLUGIN_VERSION
                REGEX "#define KDEVELOP_PLUGIN_VERSION.*" )
        if(KDEV_PLUGIN_VERSION)
            string(REGEX REPLACE ".*\\ ([0-9]+)"
                    "\\1" KDEV_PLUGIN_VERSION ${KDEV_PLUGIN_VERSION})
        else(${KDEV_PLUGIN_VERSION})
            find_file( KDEV_IPLUGINVERSION_H kdevplatform/interfaces/ipluginversion.h )
            if(KDEV_IPLUGINVERSION_H)
                    file(STRINGS ${KDEV_IPLUGINVERSION_H} KDEV_PLUGIN_VERSION
                            REGEX "#define KDEVELOP_PLUGIN_VERSION.*" )
                    string(REGEX REPLACE ".*\\ ([0-9]+)"
                            "\\1" KDEV_PLUGIN_VERSION ${KDEV_PLUGIN_VERSION})
            endif(KDEV_IPLUGINVERSION_H)
        endif(KDEV_PLUGIN_VERSION)
endif(KDEV_IPLUGIN_H)

if(NOT DEFINED KDEV_PLUGIN_VERSION)
        message(WARNING "KDEVELOP plugin version cannot be determined, guessing..." )
        set(KDEV_PLUGIN_VERSION "9")
endif(NOT DEFINED KDEV_PLUGIN_VERSION)

macro(ktl_install_desktop DESKTOP_FILE)
        configure_file( ${DESKTOP_FILE}.in ${DESKTOP_FILE} )
        install( FILES
                ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE}
                DESTINATION ${SERVICES_INSTALL_DIR} )
endmacro(ktl_install_desktop)

