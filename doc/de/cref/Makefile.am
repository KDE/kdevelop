EXTRA_DIST =index.html

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/de/kdevelop/reference/C/
	$(INSTALL_DATA) index.html $(kde_htmldir)/de/kdevelop/reference/C/cref.html

uninstall-local:
	-rm -f $(kde_htmldir)/de/kdevelop/reference/C/cref.html
