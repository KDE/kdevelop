EXTRA_DIST =index.sgml logotp3.png \
            index.html

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/hu/kdevelop/welcome
	$(INSTALL_DATA) logotp3.png $(kde_htmldir)/hu/kdevelop/welcome/logotp3.png
	$(INSTALL_DATA) index.html $(kde_htmldir)/hu/kdevelop/welcome/index.html

uninstall-local:
	-rm -f $(kde_htmldir)/hu/kdevelop/welcome/logotp3.png
	-rm -f $(kde_htmldir)/hu/kdevelop/welcome/index.html
