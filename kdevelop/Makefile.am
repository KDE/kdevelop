####### kdevelop will overwrite this part!!! (begin)##########
kdevelop_SOURCES = ctoolsconfigdlg.cpp cexecuteargdlg.cpp main.cpp ktipofday.cpp kswallow.cpp kstartuplogo.cpp cupdatekdedocdlg.cpp ctoolclass.cpp ctabctl.cpp crealfileview.cpp cproject.cpp cprjoptionsdlg.cpp cprintdlg.cpp coutputwidget.cpp cnewprojectdlg.cpp cnewfiledlg.cpp cnewclassdlg.cpp clogfileview.cpp ckdevsetupdlg.cpp ckdevinstall.cpp ckdevelop_whatsthis.cpp ckdevelop_project.cpp ckdevelop_noslot.cpp ckdevelop_kdlginit.cpp ckdevelop_init.cpp ckdevelop_classview.cpp ckdevelop.cpp ckappwizard.cpp cgrouppropertiesdlg.cpp cgeneratenewfile.cpp cfinddoctextdlg.cpp cfilepropdlg.cpp cfileprintdlg.cpp ceditwidget.cpp cdoctreepropdlg.cpp cdoctree.cpp cdocbrowser.cpp cdatepikerdlg.cpp ccreatedocdatabasedlg.cpp cconfigenscriptdlg.cpp cconfiga2psdlg.cpp cclassview.cpp cclasstooldlg.cpp caddexistingfiledlg.cpp caddclassmethoddlg.cpp caddclassattributedlg.cpp 
kdevelop_LDADD   = ./classparser/libclassparser.a ./kdlgedit/libkdlgedit.a ./kwrite/libkwrite.a -lkfile -lkfm -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET)

SUBDIRS = docs classparser kdlgedit kwrite 

EXTRA_DIST = caddclassattributedlg.cpp caddclassattributedlg.h caddclassmethoddlg.cpp caddclassmethoddlg.h caddexistingfiledlg.cpp caddexistingfiledlg.h cclasstooldlg.cpp cclasstooldlg.h cclassview.cpp cclassview.h cconfiga2psdlg.cpp cconfiga2psdlg.h cconfigenscriptdlg.cpp cconfigenscriptdlg.h ccreatedocdatabasedlg.cpp ccreatedocdatabasedlg.h cdatepikerdlg.cpp cdatepikerdlg.h cdocbrowser.cpp cdocbrowser.h cdoctree.cpp cdoctree.h cdoctreepropdlg.cpp cdoctreepropdlg.h ceditwidget.cpp ceditwidget.h cfileprintdlg.cpp cfileprintdlg.h cfilepropdlg.cpp cfilepropdlg.h cfinddoctextdlg.cpp cfinddoctextdlg.h cgeneratenewfile.cpp cgeneratenewfile.h cgrouppropertiesdlg.cpp cgrouppropertiesdlg.h ckappwizard.cpp ckappwizard.h ckdevelop.cpp ckdevelop.h ckdevelop_classview.cpp ckdevelop_init.cpp ckdevelop_kdlginit.cpp ckdevelop_noslot.cpp ckdevelop_project.cpp ckdevelop_whatsthis.cpp ckdevinstall.cpp ckdevinstall.h ckdevsetupdlg.cpp ckdevsetupdlg.h clogfileview.cpp clogfileview.h cnewclassdlg.cpp cnewclassdlg.h cnewfiledlg.cpp cnewfiledlg.h cnewprojectdlg.cpp cnewprojectdlg.h coutputwidget.cpp coutputwidget.h cprintdlg.cpp cprintdlg.h cprjoptionsdlg.cpp cprjoptionsdlg.h cproject.cpp cproject.h crealfileview.cpp crealfileview.h ctabctl.cpp ctabctl.h ctoolclass.cpp ctoolclass.h cupdatekdedocdlg.cpp cupdatekdedocdlg.h debug.h kdevelop-project.xpm kdevelop.kdelnk kdevelop.xpm kdeveloprc kstartuplogo.cpp kstartuplogo.h kswallow.cpp kswallow.h ktipofday.cpp ktipofday.h main.cpp mini-kdevelop.xpm resource.h structdef.h x-kdevelop-project.kdelnk cexecuteargdlg.cpp cexecuteargdlg.h ctoolsconfigdlg.cpp ctoolsconfigdlg.h 

####### kdevelop will overwrite this part!!! (end)############
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
SUBDIRS = tools docs templates toolbar pics kwrite kdlgedit classparser print
LFLAGS = -+
####### This part is very kdevelop specific
# you can add here more. This one gets installed 
bin_PROGRAMS = kdevelop

# Which sources should be compiled for kdevelop.
kdevelop_SOURCES = ckdevelop.cpp main.cpp cnewfiledlg.cpp coutputwidget.cpp \
	ckdevelop_init.cpp ctabctl.cpp ckdevelop_noslot.cpp ckdevelop_project.cpp \
	cclassview.cpp ceditwidget.cpp ckdevsetupdlg.cpp  ckdevelop_classview.cpp \
	cprjoptionsdlg.cpp ckappwizard.cpp crealfileview.cpp \
	clogfileview.cpp cdocbrowser.cpp cproject.cpp \
	kswallow.cpp cgeneratenewfile.cpp \
	cdoctree.cpp cnewclassdlg.cpp cfilepropdlg.cpp caddexistingfiledlg.cpp \
	cgrouppropertiesdlg.cpp cupdatekdedocdlg.cpp ccreatedocdatabasedlg.cpp \
	ctoolclass.cpp cdoctreepropdlg.cpp \
	ckdevelop_whatsthis.cpp \
	cnewprojectdlg.cpp cfinddoctextdlg.cpp kstartuplogo.cpp ktipofday.cpp \
	ckdevinstall.cpp ckdevelop_kdlginit.cpp \
	caddclassmethoddlg.cpp caddclassattributedlg.cpp \
	cclasstooldlg.cpp cclasstreehandler.cpp ctreehandler.cpp \
	ctreeview.cpp cerrormessageparser.cpp cexecuteargdlg.cpp \
	ccvaddfolderdlg.cpp klangcombo.cpp caddnewtranslationdlg.cpp grepdialog.cpp \
	ctoolsconfigdlg.cpp
	
kdevelop_METASOURCES = USE_AUTOMOC
# the library search path. 
kdevelop_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kdevelop_LDADD   = ./print/libprint.a ./kdlgedit/libkdlgedit.a ./kwrite/libkwrite.a ./classparser/libclassparser.a -lkfile -lkfm -lkspell $(LIB_KHTMLW) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET) -lfl -lgdbm

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = ckdevelop.h cnewfiledlg.h ceditwidget.h coutputwidget.h \
	ctabctl.h ckdevsetupdlg.h cprjoptionsdlg.h crealfileview.h \
	clogfileview.h structdef.h cdocbrowser.h cproject.h \
	cclassview.h kswallow.h cgeneratenewfile.h \
	cdoctree.h cnewclassdlg.h cfilepropdlg.h caddexistingfiledlg.h \
	cgrouppropertiesdlg.h cupdatekdedocdlg.h resource.h kdevelop.kdelnk \
	ckappwizard.h ccreatedocdatabasedlg.h ctoolclass.h cdoctreepropdlg.h \
	debug.h \
	cnewprojectdlg.h cfinddoctextdlg.h kstartuplogo.h ktipofday.h ckdevinstall.h \
	caddclassmethoddlg.h caddclassattributedlg.h \
	cclasstooldlg.h cclasstreehandler.h ctreehandler.h \
	ctreeview.h cerrormessageparser.h cexecuteargdlg.h \
	ccvaddfolderdlg.h klangcombo.h caddnewtranslationdlg.h grepdialog.h \
	ctoolsconfigdlg.h
	
EXTRA_DIST = kdevelop.kdelnk kdevelop.xpm mini-kdevelop.xpm kdeveloprc x-kdevelop-project.kdelnk kdevelop-project.xpm

# if you "make distclean", this files get removed. If you want to remove
# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(kdevelop_METASOURCES)

# make messages.po. Move this one to ../po/ and "make merge" in po
# the -x is for skipping messages already translated in kdelibs
messages:
	$(XGETTEXT) -C -ki18n -x $(KDEDIR)/include/kde.pot $(kdevelop_SOURCES) && mv messages.po ../po/kdevelop.pot

# just install datas here. Use install-exec-local for scripts and etc.
# the binary itself is already installed from automake
# use mkinstalldirs, not "install -d"
# don't install a list of file. Just one file per install.
# if you have more of them, create a subdirectory with an extra Makefile 
install-data-local: 
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) kdevelop.kdelnk $(APPSDIR)
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

