dataFiles = Makefile.am Example.png  app.kdevelop  app.pro exampleplugin.cpp \
	examplepluginimpl.cpp  examplepluginwidget.cpp exampleplugin.h \
	examplepluginimpl.h  examplepluginwidget.h opietoday

template_DATA = opietoday
### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz