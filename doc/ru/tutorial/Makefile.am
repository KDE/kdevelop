####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = index-1.html index-2.html index-3.html index-4.html index-5.html index-6.html index.html index.sgml logotp3.png 

####### kdevelop will overwrite this part!!! (end)############
EXTRA_DIST = index.html index-1.html index-2.html index-3.html index-4.html index-5.html index-6.html logotp3.png index.sgml

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/ru/kdevelop/tutorial
	$(INSTALL_DATA) $(srcdir)/logotp3.png $(kde_htmldir)/ru/kdevelop/tutorial/logotp3.png
	$(INSTALL_DATA) $(srcdir)/index.html $(kde_htmldir)/ru/kdevelop/tutorial/index.html
	$(INSTALL_DATA) $(srcdir)/index-1.html $(kde_htmldir)/ru/kdevelop/tutorial/index-1.html
	$(INSTALL_DATA) $(srcdir)/index-2.html $(kde_htmldir)/ru/kdevelop/tutorial/index-2.html
	$(INSTALL_DATA) $(srcdir)/index-3.html $(kde_htmldir)/ru/kdevelop/tutorial/index-3.html
	$(INSTALL_DATA) $(srcdir)/index-4.html $(kde_htmldir)/ru/kdevelop/tutorial/index-4.html
	$(INSTALL_DATA) $(srcdir)/index-5.html $(kde_htmldir)/ru/kdevelop/tutorial/index-5.html
	$(INSTALL_DATA) $(srcdir)/index-6.html $(kde_htmldir)/ru/kdevelop/tutorial/index-6.html
uninstall-local:
	-rm -f $(kde_htmldir)/ru/kdevelop/tutorial/logotp3.png
	-rm -f $(kde_htmldir)/ru/kdevelop/tutorial/index.html
	-rm -f $(kde_htmldir)/ru/kdevelop/tutorial/index-1.html
	-rm -f $(kde_htmldir)/ru/kdevelop/tutorial/index-2.html
	-rm -f $(kde_htmldir)/ru/kdevelop/tutorial/index-3.html
	-rm -f $(kde_htmldir)/ru/kdevelop/tutorial/index-4.html
	-rm -f $(kde_htmldir)/ru/kdevelop/tutorial/index-5.html
	-rm -f $(kde_htmldir)/ru/kdevelop/tutorial/index-6.html

doc: index.sgml
	ksgml2html index.sgml ru
