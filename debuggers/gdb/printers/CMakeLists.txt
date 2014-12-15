add_subdirectory(tests)

install( FILES helper.py qt4.py kde4.py gdbinit
    DESTINATION ${DATA_INSTALL_DIR}/kdevgdb/printers )
