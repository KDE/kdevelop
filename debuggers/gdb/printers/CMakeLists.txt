add_subdirectory(tests)

install( FILES helper.py qt4.py kde4.py gdbinit
    DESTINATION ${KDE_INSTALL_DATADIR}/kdevgdb/printers )
