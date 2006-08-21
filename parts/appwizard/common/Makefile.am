CLEANFILES = *.tar.gz

admin.tar.gz:
	$(TAR) -cf admin.tar -C $(srcdir) admin
	gzip admin.tar

gnome.tar.gz:
	$(TAR) -cf gnome.tar -C $(srcdir)/gnome macros
	gzip gnome.tar

gnome2.tar.gz:
	$(TAR) -cf gnome2.tar -C $(srcdir)/gnome2 macros
	gzip gnome2.tar

gnu.tar.gz:
	$(TAR) -cf gnu.tar -C $(srcdir)/gnu AUTHORS  COPYING \
		ChangeLog  INSTALL  NEWS  README  TODO
	gzip gnu.tar

wxwidgets.tar.gz:
	$(TAR) -cf wxwidgets.tar -C $(srcdir)/wx macros
	gzip wxwidgets.tar

incadmin.tar.gz:
	$(TAR) -cf incadmin.tar -C $(srcdir)/incadmin \
		config.guess  config.sub  depcomp install-sh  \
		ltmain.sh  missing  mkinstalldirs
	gzip incadmin.tar

scons.tar.gz:
	$(TAR) -cf scons.tar -C $(srcdir)/scons configure admin/scons-mini.tar.bz2 admin/generic.py admin/kde.py
	gzip scons.tar

appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
commondata_DATA = admin.tar.gz gnu.tar.gz incadmin.tar.gz gnome.tar.gz gnome2.tar.gz wxwidgets.tar.gz \
	kde-app.lsm kde-Makefile.am kde-Makefile.cvs kde-configure.in.in \
	kde-index.docbook kde-po-Makefile.am kde-doc-Makefile.am kde-doc-en-Makefile.am \
	hi16-app-app.png hi32-app-app.png kde-app.desktop kde-part.desktop \
	gnome-app.lsm gnome-Makefile.am gnome-Makefile.cvs \
	gnome-pixmaps-Makefile.am gnome-app.png \
	wx-configure.in wx-Makefile.am wx-Makefile.cvs \
	COPYING COPYING-DOCS COPYING.LIB LICENSE.QPL LICENSE.BSD Artistic \
	admin.kdevtemplate gnu.kdevtemplate dockbook.kdevtemplate\
	incadmin.kdevtemplate gnome2.kdevtemplate scons.kdevtemplate scons.tar.gz

