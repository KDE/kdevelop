####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = kdevelop
kdevelop_SOURCES = kdevlistview.cpp main.cpp ktipofday.cpp kswallow.cpp kstartuplogo.cpp klangcombo.cpp grepdialog.cpp doctreeview.cpp dbgtoolbar.cpp dbgpsdlg.cpp cupdatekdedocdlg.cpp ctreeview.cpp ctreehandler.cpp ctoolsconfigdlg.cpp ctoolclass.cpp ctabctl.cpp crealfileview.cpp cproject.cpp cprjoptionsdlg.cpp coutputwidget.cpp cnewfiledlg.cpp cnewclassdlg.cpp cmakemanualdlgdata.cpp cmakemanualdlg.cpp clogfileview.cpp ckdevsetupdlg.cpp ckdevinstall.cpp ckdevelop_whatsthis.cpp ckdevelop_project.cpp ckdevelop_noslot.cpp ckdevelop_kdlginit.cpp ckdevelop_init.cpp ckdevelop_classview.cpp ckdevelop.cpp ckdevaccel.cpp ckappwizard.cpp cgrouppropertiesdlg.cpp cgeneratenewfile.cpp cfinddoctextdlg.cpp cfilepropdlg.cpp cexecuteargdlg.cpp cerrormessageparser.cpp ceditwidget.cpp cdoctreepropdlg.cpp cdocbrowser.cpp ccvaddfolderdlg.cpp ccreatedocdatabasedlg.cpp cclassview.cpp cclasstreehandler.cpp cclasstooltreeview.cpp cclasstooldlg.cpp cbugreportdlg.cpp caddnewtranslationdlg.cpp caddexistingfiledlg.cpp caddclassmethoddlg.cpp caddclassattributedlg.cpp 
kdevelop_LDADD   = ./classparser/libclassparser.a ./classwizard/libclasswizard.a ./dbg/libdbg.a ./gfxview/libgfxview.a ./kdlgedit/libkdlgedit.a ./kwrite/libkwrite.a ./print/libprint.a ./vc/libvc.a -lkhtml -lm -lkfile -lkdeui -lkdecore $(LIB_QT) $(LIBSOCKET)

SUBDIRS = classparser classwizard dbg examples gfxview kdlgedit kwrite pics print tools vc 

EXTRA_DIST = kdevelop.desktop kdevelop.xpm mini-kdevelop.xpm caddclassattributedlg.cpp caddclassattributedlg.h caddclassmethoddlg.cpp caddclassmethoddlg.h caddexistingfiledlg.cpp caddexistingfiledlg.h caddnewtranslationdlg.cpp caddnewtranslationdlg.h cbugreportdlg.cpp cbugreportdlg.h cclasstooldlg.cpp cclasstooldlg.h cclasstooltreeview.cpp cclasstooltreeview.h cclasstreehandler.cpp cclasstreehandler.h cclassview.cpp cclassview.h ccreatedocdatabasedlg.cpp ccreatedocdatabasedlg.h ccvaddfolderdlg.cpp ccvaddfolderdlg.h cdocbrowser.cpp cdocbrowser.h cdoctreepropdlg.cpp cdoctreepropdlg.h ceditwidget.cpp ceditwidget.h cerrormessageparser.cpp cerrormessageparser.h cexecuteargdlg.cpp cexecuteargdlg.h cfilepropdlg.cpp cfilepropdlg.h cfinddoctextdlg.cpp cfinddoctextdlg.h cgeneratenewfile.cpp cgeneratenewfile.h cgrouppropertiesdlg.cpp cgrouppropertiesdlg.h ckappwizard.cpp ckappwizard.h ckdevaccel.cpp ckdevaccel.h ckdevelop.cpp ckdevelop.h ckdevelop_classview.cpp ckdevelop_init.cpp ckdevelop_kdlginit.cpp ckdevelop_noslot.cpp ckdevelop_project.cpp ckdevelop_whatsthis.cpp ckdevinstall.cpp ckdevinstall.h ckdevsetupdlg.cpp ckdevsetupdlg.h clogfileview.cpp clogfileview.h cmakemanualdlg.cpp cmakemanualdlg.h cmakemanualdlgdata.cpp cnewclassdlg.cpp cnewclassdlg.h cnewfiledlg.cpp cnewfiledlg.h coutputwidget.cpp coutputwidget.h cprjoptionsdlg.cpp cprjoptionsdlg.h cproject.cpp cproject.h crealfileview.cpp crealfileview.h ctabctl.cpp ctabctl.h ctoolclass.cpp ctoolclass.h ctoolsconfigdlg.cpp ctoolsconfigdlg.h ctreehandler.cpp ctreehandler.h ctreeview.cpp ctreeview.h cupdatekdedocdlg.cpp cupdatekdedocdlg.h dbgpsdlg.cpp dbgpsdlg.h dbgtoolbar.cpp dbgtoolbar.h debug.h development_package.xpm doctreeview.cpp doctreeview.h extractrc grepdialog.cpp grepdialog.h kdevelop-project.xpm kdevelop-setup.kdelnk kdevelop.kdelnk kdeveloprc klangcombo.cpp klangcombo.h kstartuplogo.cpp kstartuplogo.h kswallow.cpp kswallow.h ktipofday.cpp ktipofday.h main.cpp mini-development_package.xpm resource.h structdef.h x-kdevelop-project.kdelnk kdevlistview.cpp kdevlistview.h 

install-data-local:
	$(mkinstalldirs) $(kde_appsdir)/Applications/
	$(INSTALL_DATA) kdevelop.desktop $(kde_appsdir)/Applications/kdevelop.desktop
	$(mkinstalldirs) $(kde_icondir)/medium/locolor/apps/
	$(INSTALL_DATA) kdevelop.xpm $(kde_icondir)/medium/locolor/apps/kdevelop.xpm
	$(mkinstalldirs) $(kde_icondir)/small/locolor/apps/
	$(INSTALL_DATA) mini-kdevelop.xpm $(kde_icondir)/small/locolor/apps/kdevelop.xpm

uninstall-local:
	-rm -f $(kde_appsdir)/Applications/kdevelop.desktop
	-rm -f $(kde_icondir)/medium/locolor/apps/kdevelop.xpm
	-rm -f $(kde_icondir)/small/locolor/apps/kdevelop.xpm

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

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

METASOURCES = AUTO

# the library search path. 
kdevelop_LDFLAGS = $(all_libraries) $(KDE_RPATH)

rcdir = $(kde_datadir)/kdevelop
rc_DATA = kdevelopui.rc

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kdevelop.pot; \
	fi

