appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
prctooldir = ${appwizarddatadir}/template-prc-tool
templatedir = ${appwizarddatadir}/templates

prctool_DATA = prc-tool.png src-Makefile prc-tool.kdevelop callback.h palmhello.c palmhello.pbitm palmhello.rcp palmhelloRsc.h
template_DATA = prc-tool

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local


