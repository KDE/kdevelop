EXTRA_DIST = kde_projectspace.tar.gz 

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kdevelop/projectspaces/
	$(INSTALL_DATA) $(srcdir)/kde_projectspace.tar.gz $(kde_datadir)/kdevelop/projectspaces/kde_projectspace.tar.gz

uninstall-local:
	-rm -f $(kde_datadir)/kdevelop/projectspaces/kde_projectspace.tar.gz
