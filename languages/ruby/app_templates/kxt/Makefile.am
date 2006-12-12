dataFiles = app.rb main.rb prefs.rb app.cpp appview.rb appview_base.ui prefs-base.ui \
			appui.rc app.desktop kxtruby.png app.kdevelop \
			app-Makefile.am app-configure.in.in \
			app.kcfg settings.kcfgc src-Makefile.am subdirs README

templateName = kxtruby

### no need to change below:
template_DATA = $(templateName).kdevtemplate
templatedir = ${appwizarddatadir}/templates

appwizarddatadir = ${kde_datadir}/kdevappwizard
$(templateName).tar.gz: ${dataFiles}
	$(TAR) -cf $(templateName).tar -C $(srcdir) ${dataFiles}
	$(GZIP_COMMAND) -f9 $(templateName).tar

archivedir = ${appwizarddatadir}
archive_DATA = $(templateName).tar.gz ${templateName}.png

CLEANFILES = *.tar.gz
