EXTRA_DIST =index.html

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/en/kdevelop/reference/C/
	$(INSTALL_DATA) $(srcdir)/index.html $(kde_htmldir)/en/kdevelop/reference/C/cref.html

uninstall-local:
	-rm -f $(kde_htmldir)/en/kdevelop/reference/C/cref.html
