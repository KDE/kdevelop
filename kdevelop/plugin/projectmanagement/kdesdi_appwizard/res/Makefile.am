
EXTRA_DIST = kdesdi.tar.gz normalApp.bmp 

install-data-local:
	$(mkinstalldirs) ${kde_datadir}/kdevelop/project_templates/
	$(INSTALL_DATA) kdesdi.tar.gz ${kde_datadir}/kdevelop/project_templates/kdesdi.tar.gz
	$(mkinstalldirs) ${kde_datadir}/kdevelop/appwizard_pics/
	$(INSTALL_DATA) normalApp.bmp ${kde_datadir}/kdevelop/appwizard_pics/normalApp.bmp

uninstall-local:
	-rm -f ${kde_datadir}/kdevelop/project_templates/kdesdi.tar.gz
	-rm -f ${kde_datadir}/kdevelop/appwizard_pics/normalApp.bmp

