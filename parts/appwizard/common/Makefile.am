admin.tar.gz:
	tar -C $(srcdir) -cf admin.tar admin/
	gzip admin.tar

gnome.tar.gz:
	tar -C $(srcdir)/gnome -cf gnome.tar macros/
	gzip gnome.tar

gnu.tar.gz:
	tar -C $(srcdir)/gnu -cf gnu.tar AUTHORS  COPYING \
		ChangeLog  INSTALL  NEWS  README  TODO
	gzip gnu.tar

incadmin.tar.gz:
	tar -C $(srcdir)/incadmin -cf incadmin.tar acconfig.h  \
		config.guess  config.sub  depcomp install-sh  \
		ltconfig  ltmain.sh  missing  mkinstalldirs
	gzip incadmin.tar

appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
commondata_DATA = gideon.pm admin.tar.gz gnu.tar.gz incadmin.tar.gz gnome.tar.gz \
		  kde-app.lsm kde-Makefile.am kde-Makefile.cvs kde-configure.in.in \
		  kde-index.docbook kde-po-Makefile.am kde-doc-Makefile.am kde-doc-en-Makefile.am \
		  hi16-app-app.png hi32-app-app.png kde-app.desktop kde-part.desktop \
		  gnome-app.lsm gnome-Makefile.am gnome-Makefile.cvs \
		  gnome-pixmaps-Makefile.am gnome-app.png \
		  COPYING COPYING.LIB LICENSE.QPL LICENSE.BSD Artistic
