appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
pytkdir = ${appwizarddatadir}/template-pytk
templatedir = ${appwizarddatadir}/templates

pytk_DATA = app.py app.kdevelop pytk.png
template_DATA = pytk

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(pytkdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(pytkdir)/script
