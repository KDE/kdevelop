
# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.desktop) should go to. 
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)
# claim, which subdirectories you want to install
# if you don't have subdirectories, comment it
SUBDIRS = tools templates toolbar pics kwrite classparser print vc gfxview classwizard plugins widgets dlgedit
LFLAGS = -+
####### This part is very kdevelop specific
# you can add here more. This one gets installed 
bin_PROGRAMS = kdevelop

# Which sources should be compiled for kdevelop.
kdevelop_SOURCES = klistview.cpp ckdevelop.cpp main.cpp cnewfiledlg.cpp \
	ckdevelop_init.cpp ctabctl.cpp ckdevelop_noslot.cpp ckdevelop_project.cpp \
	cclassview.cpp ceditwidget.cpp ckdevsetupdlg.cpp  ckdevelop_classview.cpp \
	cprjoptionsdlg.cpp ckappwizard.cpp crealfileview.cpp \
	clogfileview.cpp cdocbrowser.cpp cproject.cpp \
	cgeneratenewfile.cpp componentmanager.cpp \
	doctreeview.cpp cnewclassdlg.cpp cfilepropdlg.cpp caddexistingfiledlg.cpp \
	cgrouppropertiesdlg.cpp cupdatekdedocdlg.cpp ccreatedocdatabasedlg.cpp \
	ctoolclass.cpp cdoctreepropdlg.cpp grepviewconfigwidget.cpp \
	ckdevelop_whatsthis.cpp mdiframe.cpp \
	 cfinddoctextdlg.cpp kstartuplogo.cpp ktipofday.cpp \
	ckdevinstall.cpp ckdevelop_kdlginit.cpp doctreeviewconfigwidget.cpp \
	caddclassmethoddlg.cpp caddclassattributedlg.cpp \
	cclasstooldlg.cpp cclasstreehandler.cpp ctreehandler.cpp \
	ctreeview.cpp cexecuteargdlg.cpp customizedlg.cpp \
	ccvaddfolderdlg.cpp klangcombo.cpp caddnewtranslationdlg.cpp \
	ctoolsconfigdlg.cpp  cclasstooltreeview.cpp component.cpp processview.cpp \
	cmakemanualdlg.cpp cmakemanualdlgdata.cpp misc.cpp cbugreportdlg.cpp kdevplugin.cpp \
	grepview.cpp makeview.cpp grepdlg.cpp outputview.cpp ckdevaccel.cpp editorview.cpp docbrowserview.cpp \
	kdevelopifaceimpl.cpp KDevelopIface.skel fileclosealldlg.cpp

kdevelop_METASOURCES = AUTO
# the library search path. 
kdevelop_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.

kdevelop_LDADD   = ./widgets/dockwidgets/libdockwidgets.a ./widgets/qextmdi/libqextmdi.a ./plugins/libkdevplugin.a ./vc/libvc.a ./print/libprint.a \
	./gfxview/libgfxview.a ./kwrite/libkwrite.a \
	./classparser/libclassparser.a ./classwizard/libclasswizard.a ./dlgedit/libdlgedit.a \
	$(LIB_KHTML) -lkspell -lkfile -lkio -lkdeui -lkdecore -lqt -lXext -lX11 -lfl -ldl $(LIBSOCKET) 

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = ckdevelop.h cnewfiledlg.h ceditwidget.h \
	ctabctl.h ckdevsetupdlg.h cprjoptionsdlg.h crealfileview.h \
	clogfileview.h structdef.h cdocbrowser.h cproject.h \
	cclassview.h  cgeneratenewfile.h componentmanager.h \
	doctreeview.h cnewclassdlg.h cfilepropdlg.h caddexistingfiledlg.h \
	cgrouppropertiesdlg.h cupdatekdedocdlg.h resource.h \
	ckappwizard.h ccreatedocdatabasedlg.h ctoolclass.h cdoctreepropdlg.h \
	cfinddoctextdlg.h kstartuplogo.h ktipofday.h ckdevinstall.h \
	caddclassmethoddlg.h caddclassattributedlg.h grepviewconfigwidget.h \
	cclasstooldlg.h cclasstreehandler.h ctreehandler.h doctreeviewconfigwidget.h \
	ctreeview.h cexecuteargdlg.h mdiframe.h customizedlg.h \
	ccvaddfolderdlg.h klangcombo.h caddnewtranslationdlg.h \
	ctoolsconfigdlg.h cbugreportdlg.h cclasstooltreeview.h \
	cmakemanualdlg.h klistview.h component.h processview.h grepview.h \
	makeview.h grepdlg.h outputview.h ckdevaccel.h editorview.h docbrowserview.h fileclosealldlg.h

EXTRA_DIST = kdeveloprc \
	.directory kdevelop.desktop x-kdevelop-project.desktop kdevelop-setup.desktop \
	los-app-development_package.png lom-app-kdevelop.png los-app-kdevelop.png lom-app-kdevelop_project.png  	

# make messages.po. Move this one to ../po/ and "make merge" in po
# the -x is for skipping messages already translated in kdelibs
messages:
	$(XGETTEXT) -C -kI18N_NOOP -ki18n -x $(includedir)/kde.pot `find . -name \*.h -o -name \*.cpp -o -name \*.cc` && mv messages.po $(podir)/kdevelop.pot 

appsdir   = $(kde_appsdir)/Applications/Development
KDE_ICON  = kdevelop kdevelop_project development_package
apps_DATA = kdevelop.desktop kdevelop-setup.desktop .directory

confdir   = $(kde_confdir)
conf_DATA = kdeveloprc

mimedir   = $(kde_mimedir)/application
mime_DATA = x-kdevelop-project.desktop
