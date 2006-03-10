dataFiles = configure.in include-kdev-pg-allocator.h include-kdev-pg-list.h \
	include-kdev-pg-memory-pool.h include-kdev-pg-replacement.h \
	include-kdev-pg-token-stream.h kdevpg.kdevtemplate kdev-pg-parser.kdevelop \
	kdevpg.png topdir-Makefile.am Makefile.cvs README.devel rec-check.sh \
	src-decoder.cpp src-decoder.h src-language.g src-language_lexer.ll \
	src-main.cpp src-Makefile.am src-tests-t001.t src-tests-t002.t

templateName = kdevpg

### no need to change below:
template_DATA = $(templateName).kdevtemplate
templatedir = ${appwizarddatadir}/templates

appwizarddatadir = ${kde_datadir}/kdevappwizard
$(templateName).tar.gz: ${dataFiles}
	$(TAR) -cf $(templateName).tar -C $(srcdir) ${dataFiles}
	$(GZIP) -f9 $(templateName).tar

archivedir = ${appwizarddatadir}
archive_DATA = $(templateName).tar.gz ${templateName}.png

CLEANFILES = *.tar.gz