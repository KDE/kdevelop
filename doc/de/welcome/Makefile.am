EXTRA_DIST =index.sgml logotp3.png \
            index.html

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/de/kdevelop/welcome
	$(INSTALL_DATA) $(srcdir)/logotp3.png $(kde_htmldir)/de/kdevelop/welcome/logotp3.png
	$(INSTALL_DATA) $(srcdir)/index.html $(kde_htmldir)/de/kdevelop/welcome/index.html

uninstall-local:
	-rm -f $(kde_htmldir)/de/kdevelop/welcome/logotp3.png
	-rm -f $(kde_htmldir)/de/kdevelop/welcome/index.html
