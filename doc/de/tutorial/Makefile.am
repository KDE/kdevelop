EXTRA_DIST = index.sgml logotp3.png 

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/de/kdevelop/tutorial
	$(INSTALL_DATA) logotp3.png $(kde_htmldir)/de/kdevelop/tutorial/logotp3.png
	$(INSTALL_DATA) index.html $(kde_htmldir)/de/kdevelop/tutorial/index.html

uninstall-local:
	-rm -f $(kde_htmldir)/de/kdevelop/tutorial/logotp3.png
