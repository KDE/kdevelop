appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
pyqtdir = ${appwizarddatadir}/template-pyqt
templatedir = ${appwizarddatadir}/templates

pyqt_DATA = app.py app.kdevelop fileopen.pyxpm fileprint.pyxpm filesave.pyxpm app-Makefile.am app-autogen.sh app-configure.ac documentdetails.ui missing install-sh mkinstalldirs pyqt-am-preproc.py
template_DATA = pyqt

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(pyqtdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(pyqtdir)/script
