EXTRA_DIST = index.sgml logotp3.png \
            index.html index-1.html index-2.html index-3.html index-4.html

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/en/kdevelop/tutorial
	$(INSTALL_DATA) logotp3.png $(kde_htmldir)/en/kdevelop/tutorial/logotp3.png
	$(INSTALL_DATA) index.html $(kde_htmldir)/en/kdevelop/tutorial/index.html
	$(INSTALL_DATA) index-1.html $(kde_htmldir)/en/kdevelop/tutorial/index-1.html
	$(INSTALL_DATA) index-2.html $(kde_htmldir)/en/kdevelop/tutorial/index-2.html
	$(INSTALL_DATA) index-3.html $(kde_htmldir)/en/kdevelop/tutorial/index-3.html
	$(INSTALL_DATA) index-4.html $(kde_htmldir)/en/kdevelop/tutorial/index-4.html

uninstall-local:
	-rm -f $(kde_htmldir)/en/kdevelop/tutorial/logotp3.png
	-rm -f $(kde_htmldir)/en/kdevelop/tutorial/index.html
	-rm -f $(kde_htmldir)/en/kdevelop/tutorial/index-1.html
	-rm -f $(kde_htmldir)/en/kdevelop/tutorial/index-2.html
	-rm -f $(kde_htmldir)/en/kdevelop/tutorial/index-3.html
	-rm -f $(kde_htmldir)/en/kdevelop/tutorial/index-4.html
