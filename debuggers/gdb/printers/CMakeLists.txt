if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

install( FILES helper.py qt.py kde.py gdbinit
    DESTINATION ${KDE_INSTALL_DATADIR}/kdevgdb/printers )
