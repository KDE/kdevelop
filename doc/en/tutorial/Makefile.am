EXTRA_DIST = index.sgml logotp3.png \
            index.html index-1.html index-2.html index-3.html index-4.html

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/kdevelop/tutorial
	$(INSTALL_DATA) logotp3.png $(DESTDIR)$(kde_htmldir)/en/kdevelop/tutorial/logotp3.png
	$(INSTALL_DATA) index.html $(DESTDIR)$(kde_htmldir)/en/kdevelop/tutorial/index.html
	$(INSTALL_DATA) index-1.html $(DESTDIR)$(kde_htmldir)/en/kdevelop/tutorial/index-1.html
	$(INSTALL_DATA) index-2.html $(DESTDIR)$(kde_htmldir)/en/kdevelop/tutorial/index-2.html
	$(INSTALL_DATA) index-3.html $(DESTDIR)$(kde_htmldir)/en/kdevelop/tutorial/index-3.html
	$(INSTALL_DATA) index-4.html $(DESTDIR)$(kde_htmldir)/en/kdevelop/tutorial/index-4.html

uninstall-local:
	-rm -f $(kde_htmldir)/en/kdevelop/tutorial/logotp3.png
	-rm -f $(kde_htmldir)/en/kdevelop/tutorial/index.html
	-rm -f $(kde_htmldir)/en/kdevelop/tutorial/index-1.html
	-rm -f $(kde_htmldir)/en/kdevelop/tutorial/index-2.html
	-rm -f $(kde_htmldir)/en/kdevelop/tutorial/index-3.html
	-rm -f $(kde_htmldir)/en/kdevelop/tutorial/index-4.html