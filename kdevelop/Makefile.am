
# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to. 
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# just set the variable
APPSDIR = $(kde_appsdir)/Applications
# set the include path for X, qt and KDE
INCLUDES= $(all_includes)
# claim, which subdirectories you want to install
# if you don't have subdirectories, comment it
SUBDIRS = tools docs templates toolbar pics kwrite kdlgedit classparser print vc gfxview classwizard
LFLAGS = -+
####### This part is very kdevelop specific
# you can add here more. This one gets installed 
bin_PROGRAMS = kdevelop

# Which sources should be compiled for kdevelop.
kdevelop_SOURCES = klistview.cpp ckdevelop.cpp main.cpp cnewfiledlg.cpp coutputwidget.cpp \
	ckdevelop_init.cpp ctabctl.cpp ckdevelop_noslot.cpp ckdevelop_project.cpp \
	cclassview.cpp ceditwidget.cpp ckdevsetupdlg.cpp  ckdevelop_classview.cpp \
	cprjoptionsdlg.cpp ckappwizard.cpp crealfileview.cpp \
	clogfileview.cpp cdocbrowser.cpp cproject.cpp \
	kswallow.cpp cgeneratenewfile.cpp \
	doctreeview.cpp cnewclassdlg.cpp cfilepropdlg.cpp caddexistingfiledlg.cpp \
	cgrouppropertiesdlg.cpp cupdatekdedocdlg.cpp ccreatedocdatabasedlg.cpp \
	ctoolclass.cpp cdoctreepropdlg.cpp \
	ckdevelop_whatsthis.cpp \
	 cfinddoctextdlg.cpp kstartuplogo.cpp ktipofday.cpp \
	ckdevinstall.cpp ckdevelop_kdlginit.cpp \
	caddclassmethoddlg.cpp caddclassattributedlg.cpp \
	cclasstooldlg.cpp cclasstreehandler.cpp ctreehandler.cpp \
	ctreeview.cpp cerrormessageparser.cpp cexecuteargdlg.cpp \
	ccvaddfolderdlg.cpp klangcombo.cpp caddnewtranslationdlg.cpp grepdialog.cpp \
	ctoolsconfigdlg.cpp cbugreportdlg.cpp cclasstooltreeview.cpp \
	cctviewdefinitioncmd.cpp cctviewdeclarationcmd.cpp 

kdevelop_METASOURCES = USE_AUTOMOC
# the library search path. 
kdevelop_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kdevelop_LDADD   = ./vc/libvc.a ./print/libprint.a ./gfxview/libgfxview.a ./kdlgedit/libkdlgedit.a ./kwrite/libkwrite.a ./classparser/libclassparser.a ./classwizard/libclasswizard.a -lkfile -lkfm -lkspell $(LIB_KHTMLW) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) -lfl -lgdbm -ldl

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = ckdevelop.h cnewfiledlg.h ceditwidget.h coutputwidget.h \
	ctabctl.h ckdevsetupdlg.h cprjoptionsdlg.h crealfileview.h \
	clogfileview.h structdef.h cdocbrowser.h cproject.h \
	cclassview.h kswallow.h cgeneratenewfile.h \
	doctreeview.h cnewclassdlg.h cfilepropdlg.h caddexistingfiledlg.h \
	cgrouppropertiesdlg.h cupdatekdedocdlg.h resource.h kdevelop.kdelnk \
	ckappwizard.h ccreatedocdatabasedlg.h ctoolclass.h cdoctreepropdlg.h \
	debug.h \
	cfinddoctextdlg.h kstartuplogo.h ktipofday.h ckdevinstall.h \
	caddclassmethoddlg.h caddclassattributedlg.h \
	cclasstooldlg.h cclasstreehandler.h ctreehandler.h \
	ctreeview.h cerrormessageparser.h cexecuteargdlg.h \
	ccvaddfolderdlg.h klangcombo.h caddnewtranslationdlg.h grepdialog.h \
	ctoolsconfigdlg.h cbugreportdlg.h cclasstooltreeview.h \
	cctviewdefinitioncmd.h cctviewdeclarationcmd.h ccommand.h

EXTRA_DIST = kdevelop.kdelnk kdevelop.xpm mini-kdevelop.xpm kdeveloprc x-kdevelop-project.kdelnk kdevelop-project.xpm .directory kdevelop-setup.kdelnk

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(kdevelop_METASOURCES)

# make messages.po. Move this one to ../po/ and "make merge" in po
# the -x is for skipping messages already translated in kdelibs
messages:
	$(XGETTEXT) -C -ki18n -x $(KDEDIR)/include/kde.pot `find . -name \*.h -o -name \*.cpp -o -name \*.cc`  && mv messages.po ../po/kdevelop.pot 

# just install datas here. Use install-exec-local for scripts and etc.
# the binary itself is already installed from automake
# use mkinstalldirs, not "install -d"
# don't install a list of file. Just one file per install.
# if you have more of them, create a subdirectory with an extra Makefile 
install-data-local: 
	$(mkinstalldirs) $(APPSDIR)/kdevelop
	$(INSTALL_DATA) kdevelop.kdelnk $(APPSDIR)/kdevelop
	$(INSTALL_DATA) kdevelop-setup.kdelnk $(APPSDIR)/kdevelop
	$(INSTALL_DATA) .directory $(APPSDIR)/kdevelop
	$(mkinstalldirs) $(kde_icondir)
	$(INSTALL_DATA) kdevelop.xpm $(kde_icondir)
	$(INSTALL_DATA) kdevelop-project.xpm $(kde_icondir)	
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-kdevelop.xpm $(kde_minidir)/kdevelop.xpm
	$(mkinstalldirs) $(kde_confdir)
	$(INSTALL_DATA) kdeveloprc $(kde_confdir)/kdeveloprc
	$(mkinstalldirs) $(kde_mimedir)/application	
	$(INSTALL_DATA) x-kdevelop-project.kdelnk $(kde_mimedir)/application/

# remove ALL you have installed in install-data-local or install-exec-local
uninstall-local:
	-rm -f $(APPSDIR)/kdevelop.kdelnk
	-rm -f $(kde_icondir)/kdevelop.xpm
	-rm -f $(kde_minidir)/kdevelop.xpm

