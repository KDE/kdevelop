
EXTRA_DIST = kdeobjcsdi.tar.gz normalApp.bmp

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kdevelop/project_templates/
	$(INSTALL_DATA) $(srcdir)/kdeobjcsdi.tar.gz $(kde_datadir)/kdevelop/project_templates/kdeobjcsdi.tar.gz
	$(mkinstalldirs) $(kde_datadir)/kdevelop/appwizard_pics/
	$(INSTALL_DATA) $(srcdir)/normalApp.bmp $(kde_datadir)/kdevelop/appwizard_pics/normalApp.bmp

uninstall-local:
	-rm -f $(kde_datadir)/kdevelop/project_templates/kdeobjcsdi.tar.gz
	-rm -f $(kde_datadir)/kdevelop/appwizard_pics/normalApp.bmp

