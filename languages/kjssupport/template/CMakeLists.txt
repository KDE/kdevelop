

#original Makefile.am contents follow:

#appwizarddatadir = ${kde_datadir}/kdevappwizard
#commondatadir = ${appwizarddatadir}/template-common
#jshellodir = ${appwizarddatadir}/template-jshello
#templatedir = ${appwizarddatadir}/templates
#
#jshello_DATA = app.js app.kdevelop
#template_DATA = jshello
#
#DISTCLEANFILES = script.local
#EXTRA_DIST = script
#
#perl = perl
#
#script.local: ${srcdir}/script
#	cp ${srcdir}/script script.local ; \
#	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local
#
#install-data-local: script.local
#	$(mkinstalldirs) $(DESTDIR)$(jshellodir)
#	$(INSTALL_DATA) script.local $(DESTDIR)$(jshellodir)/script

#######################
#previous CMakeLists.txt contents follow:

