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
SUBDIRS = tools docs templates toolbar pics kwrite

####### This part is very kdevelop specific
# you can add here more. This one gets installed 
bin_PROGRAMS = kdevelop
#CXXFLAGS = -O2 -Wall
# Which sources should be compiled for kdevelop.
kdevelop_SOURCES = ckdevelop.cpp main.cpp cnewfiledlg.cpp coutputwidget.cpp \
		   ckdevelop_init.cpp ctabctl.cpp ckdevelop_noslot.cpp ckdevelop_project.cpp \
		   cclassview.cpp ceditwidget.cpp ckdevsetupdlg.cpp  ckdevelop_classview.cpp \
		   cprjoptionsdlg.cpp ckappwizard.cpp crealfileview.cpp \
		   clogfileview.cpp cdocbrowser.cpp cproject.cpp \
		   kswallow.cpp cgeneratenewfile.cpp \
		   cdoctree.cpp cnewclassdlg.cpp cfilepropdlg.cpp caddexistingfiledlg.cpp cgrouppropertiesdlg.cpp cupdatekdedocdlg.cpp ccreatedocdatabasedlg.cpp ctoolclass.cpp cdoctreepropdlg.cpp


kdevelop_METASOURCES = USE_AUTOMOC
# the library search path. 
kdevelop_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.
kdevelop_LDADD   = ./kwrite/libkwrite.a -lkfile -lkfm $(LIB_KHTMLW) -lkdeui -lkdecore -lqt -lXext -lX11 $(LIBSOCKET)

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = ckdevelop.h cnewfiledlg.h ceditwidget.h coutputwidget.h \
		ctabctl.h ckdevsetupdlg.h cprjoptionsdlg.h crealfileview.h \
		clogfileview.h structdef.h cdocbrowser.h cproject.h \
		cclassview.h kswallow.h cgeneratenewfile.h \
		cdoctree.h cnewclassdlg.h cfilepropdlg.h caddexistingfiledlg.h cgrouppropertiesdlg.h cupdatekdedocdlg.h resource.h kdevelop.kdelnk ckappwizard.h ccreatedocdatabasedlg.h ctoolclass.h cdoctreepropdlg.h

EXTRA_DIST = kdevelop.kdelnk kdevelop.xpm mini-kdevelop.xpm kdeveloprc

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
	$(mkinstalldirs) $(kde_minidir)
	$(INSTALL_DATA) mini-kdevelop.xpm $(kde_minidir)/kdevelop.xpm
	$(mkinstalldirs) $(kde_confdir)
	$(INSTALL_DATA) kdeveloprc $(kde_confdir)/kdeveloprc

# remove ALL you have installed in install-data-local or install-exec-local
uninstall-local:
	-rm -f $(APPSDIR)/kdevelop.kdelnk
	-rm -f $(kde_icondir)/kdevelop.xpm
	-rm -f $(kde_minidir)/kdevelop.xpm






