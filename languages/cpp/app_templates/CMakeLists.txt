

set(apptemplate_DIRS
    kapp kpartapp kpartplugin kcmodule kicker kioslave kdevpart kdevpart2 
    chello cpphello gnomeapp kscreensaver 
#    kofficepart chello_gba
    dcopservice 
    konqnavpanel qmakesimple qmakeapp khello khello2 kateplugin kateplugin2 
    qtopiaapp kdedcop wxhello cppsdlhello kdevlang noatunvisual noatunui 
    kbearplugin kbearimportfilter kmod kfileplugin clanlib opieapp opieapplet 
    opieinput opiemenu opienet opietoday kxt gtk2mmapp cppcurseshello 
    win32hello win32gui cmakesimple cmakesimplec cmakelibcpp cmakelibc kscons_kxt kscons_kmdi
)

kdevelop_add_app_templates(${apptemplate_DIRS})

########### install files ###############

install_files(${DATA_INSTALL_DIR}/kdevelop/profiles/IDE/CompiledLanguageIDE/CandCppIDE/CppIDE/KDECppIDE FILES kdecpp.appwizard)
install_files(${DATA_INSTALL_DIR}/kdevelop/profiles/IDE/CompiledLanguageIDE/CandCppIDE/CppIDE FILES cpp.appwizard)
install_files(${DATA_INSTALL_DIR}/kdevelop/profiles/IDE/CompiledLanguageIDE/CandCppIDE/CIDE FILES c.appwizard)


