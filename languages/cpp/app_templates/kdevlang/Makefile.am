appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kdevlangdir = ${appwizarddatadir}/template-kdevlang
templatedir = ${appwizarddatadir}/templates

kdevlang_DATA = src-Makefile.am kdevlang_part.cpp kdevlang_part.h kdevlang.rc kdevlang.desktop app.kdevelop kdevlang-configure.in.in README.dox
template_DATA = kdevlang

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(kdevlangdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(kdevlangdir)/script
