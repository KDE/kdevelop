ecm_add_test(svnrecursiveadd.cpp LINK_LIBRARIES
    Qt5::Test Qt5::Gui
    KDev::Tests KDev::Util KDev::Vcs
)

ecm_add_test(svnimport.cpp LINK_LIBRARIES
    Qt5::Test Qt5::Gui
    KDev::Tests KDev::Util KDev::Vcs
)
