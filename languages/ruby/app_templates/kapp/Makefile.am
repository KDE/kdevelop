dataFiles = app_client.rb appiface.rb app.rb appview.rb main.rb pref.rb app.cpp  \
			appui.rc app.desktop app.kdevelop app-Makefile.am app-configure.in.in \
			src-Makefile.am subdirs README

templateName = kappruby

### no need to change below:
template_DATA = $(templateName).kdevtemplate
templatedir = ${appwizarddatadir}/templates

appwizarddatadir = ${kde_datadir}/kdevappwizard
$(templateName).tar.gz:
	$(TAR) -cf $(templateName).tar -C $(srcdir) ${dataFiles}
	$(GZIP) -9 $(templateName).tar

archivedir = ${appwizarddatadir}
archive_DATA = $(templateName).tar.gz ${templateName}.png

CLEANFILES = *.tar.gz
