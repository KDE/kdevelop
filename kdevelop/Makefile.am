###### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = kdevelop

## INCLUDES were found outside kdevelop specific part

kdevelop_SOURCES = caddtemplatedialog.cpp ccodetemplateopts.cpp kdevcodetemplate.cpp addtemplatedialog.ui codetemplateoptsdlg.ui tokenizer.cpp classparser.cpp ccompletionopts.cpp ccompletionoptsdlg.ui simpleparser.cpp kdevregexp.cpp cprjconfchange.cpp cprjcompoptsdlg.ui cprjcompopts.cpp cprjaddoptsdlg.ui cprjaddopts.cpp kdevcodecompletion.cpp ccompconf.cpp ccompconfdlg.ui ctoolclass.cpp kdevsession.cpp clibpropdlgimpl.cpp clibpropdlg.ui ckonsolewidget.cpp docviewman.cpp ckdevelop.cpp ckdevelop_classview.cpp ckdevelop_noslot.cpp cprjoptionsdlg.cpp ckappwizard.cpp wzconnectdlg.ui ckdevelop_init.cpp main.cpp kstartuplogo.cpp kdevlistview.cpp grepdialog.cpp doctreeview.cpp dbgtoolbar.cpp dbgpsdlg.cpp ctreeview.cpp ctreehandler.cpp ctoolsconfigdlg.cpp crealfileview.cpp cproject.cpp coutputwidget.cpp cnewfiledlg.cpp cnewclassdlg.cpp cmakemanualdlgdata.cpp cmakemanualdlg.cpp clogfileview.cpp ckdevsetupdlg.cpp ckdevelop_whatsthis.cpp ckdevelop_project.cpp cclasstreehandler.cpp ckdevaccel.cpp cgrouppropertiesdlg.cpp cgeneratenewfile.cpp cfinddoctextdlg.cpp cfilepropdlg.cpp cexecuteargdlg.cpp cdoctreepropdlg.cpp cdocbrowser.cpp ccvaddfolderdlg.cpp cclonefunctiondlg.cpp cclassview.cpp wzconnectdlgimpl.cpp cclasstooltreeview.cpp cclasstooldlg.cpp caddnewtranslationdlg.cpp caddexistingfiledlg.cpp caddclassmethoddlg.cpp caddclassattributedlg.cpp api.cpp core.cpp partcontroller.cpp plugincontroller.cpp partselectwidget.cpp kdevprojectimpl.cpp projectmanager.cpp
kdevelop_LDADD   = ./vc/libkdevelopvc.la ./gfxview/libkdevelopgfxview.la ./dbg/libkdevelopdbg.la ./classwizard/libclasswizard.a ./kpp/libkdevelopkpp.la ./ctags/libctags.a ./setup/libkdevelopsetup.la $(top_builddir)/lib/libkdevelop.la $(LEXLIB) -lktexteditor -lkatepartinterfaces $(LIB_KFILE) $(LIB_KHTML) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)

SUBDIRS = kpp classwizard dbg examples gfxview pics templates tools vc ctags setup 

EXTRA_DIST = caddclassattributedlg.cpp caddclassattributedlg.h caddclassmethoddlg.cpp caddclassmethoddlg.h caddexistingfiledlg.cpp caddexistingfiledlg.h caddnewtranslationdlg.cpp caddnewtranslationdlg.h cclasstooldlg.cpp cclasstooldlg.h cclasstooltreeview.cpp cclasstooltreeview.h wzconnectdlgimpl.cpp cclasstreehandler.h cclassview.cpp cclassview.h cclonefunctiondlg.cpp cclonefunctiondlg.h ccvaddfolderdlg.cpp ccvaddfolderdlg.h cdocbrowser.cpp cdocbrowser.h cdoctreepropdlg.cpp cdoctreepropdlg.h wzconnectdlgimpl.h cexecuteargdlg.cpp cexecuteargdlg.h cfilepropdlg.cpp cfilepropdlg.h cfinddoctextdlg.cpp cfinddoctextdlg.h cgeneratenewfile.cpp cgeneratenewfile.h cgrouppropertiesdlg.cpp cgrouppropertiesdlg.h ckdevaccel.h ckdevaccel.cpp cclasstreehandler.cpp ckdevelop.h ckdevelop_project.cpp ckdevelop_whatsthis.cpp ckdevsetupdlg.cpp ckdevsetupdlg.h clogfileview.cpp clogfileview.h cmakemanualdlg.cpp cmakemanualdlg.h cmakemanualdlgdata.cpp cnewclassdlg.cpp cnewclassdlg.h cnewfiledlg.cpp cnewfiledlg.h coutputwidget.cpp coutputwidget.h cprjoptionsdlg.h cproject.cpp cproject.h crealfileview.cpp crealfileview.h ctoolsconfigdlg.cpp ctoolsconfigdlg.h ctreehandler.cpp ctreehandler.h ctreeview.cpp ctreeview.h dbgpsdlg.cpp dbgpsdlg.h dbgtoolbar.cpp dbgtoolbar.h debug.h doctreeview.cpp doctreeview.h extractrc grepdialog.cpp grepdialog.h kdevelop-setup.desktop kdevelop.desktop kdeveloprc kdevelopui.rc kdevlistview.cpp kdevlistview.h kstartuplogo.cpp kstartuplogo.h main.cpp resource.h structdef.h ckdevelop_init.cpp wzconnectdlg.ui x-kdevelop-project.desktop ckappwizard.cpp ckappwizard.h cprjoptionsdlg.cpp ckdevelop_noslot.cpp ckdevelop_classview.cpp ckdevelop.cpp hi16-app-kdevelop.png hi32-app-kdevelop.png hi48-app-kdevelop.png docviewman.cpp docviewman.h ckonsolewidget.cpp ckonsolewidget.h clibpropdlg.ui clibpropdlgimpl.cpp clibpropdlgimpl.h kdevsession.cpp kdevsession.h ctoolclass.cpp ctoolclass.h ccompconfdlg.ui ccompconf.h ccompconf.cpp kdevcodecompletion.cpp kdevcodecompletion.h cprjaddopts.cpp cprjaddopts.h cprjaddoptsdlg.ui cprjcompopts.cpp cprjcompopts.h cprjcompoptsdlg.ui cprjconfchange.cpp cprjconfchange.h kdevregexp.cpp kdevregexp.h simpleparser.cpp simpleparser.h ccompletionoptsdlg.ui ccompletionopts.cpp ccompletionopts.h tips classparser.cpp classparser.h tokenizer.cpp tokenizer.h codetemplateoptsdlg.ui addtemplatedialog.ui kdevcodetemplate.cpp kdevcodetemplate.h ccodetemplateopts.cpp ccodetemplateopts.h caddtemplatedialog.cpp caddtemplatedialog.h 

####### kdevelop will overwrite this part!!! (end)############

INCLUDES = -I$(top_srcdir)/lib/qextmdi/include -I$(top_builddir)/kdevelop/ctags -I$(top_builddir)/kdevelop/kpp -I$(top_srcdir)/kdevelop/kpp -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/sourceinfo -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/structure $(all_includes)

kdevelop_LDADD += \
	$(top_builddir)/lib/interfaces/libkdevinterfaces.la \
	$(top_builddir)/lib/qextmdi/src/libkdevelopqextmdi.la \
	$(top_builddir)/lib/libkdevelop.la

#        $(top_builddir)/parts/classview/libkdevclassview.la \
#        $(top_builddir)/parts/grepview/libkdevgrepview.la \
#        $(top_builddir)/parts/appwizard/libkdevappwizard.la \
#        $(top_builddir)/parts/outputviews/libkdevoutputviews.la

tokenizer.cpp: tokenizer.l
	flex -+ -otokenizer.cpp $(srcdir)/tokenizer.l && cat tokenizer.cpp | \
	sed -e "s,class istream;,#include <iostream\.h>," > tokenizer.cpp.tmp && \
	mv tokenizer.cpp.tmp tokenizer.cpp

METASOURCES = AUTO

# the library search path.
kdevelop_LDFLAGS = $(all_libraries) $(KDE_RPATH)

rcdir = $(kde_datadir)/kdevelop
rc_DATA = kdevelopui.rc

KDE_ICON = AUTO
KDE_OPTIONS = nofinal

data_DATA = kdevelop.desktop kdevelop-setup.desktop
datadir = $(kde_appsdir)/Development

tip_DATA = tips
tipdir = $(kde_datadir)/kdevelop

templates_DATA = templates.xml
templatesdir = $(kde_datadir)/kdevelop

mime_DATA = x-kdevelop-project.desktop
mimedir = $(kde_mimedir)/application

bin_SCRIPTS = extractrc

messages: rc.cpp
	$(PREPARETIPS) > tips.cc
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) -ktr $$LIST -o $(podir)/kdevelop.pot; \
	fi
	rm -f tips.cc
