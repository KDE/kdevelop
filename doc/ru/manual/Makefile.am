####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = index-1.html index-2.html index-3.html index-4.html index-5.html index-6.html index.html 

####### kdevelop will overwrite this part!!! (end)############
EXTRA_DIST = index.html index-1.html index-2.html index-3.html index-4.html index-5.html index-6.html
		
install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/ru/kdevelop
	$(INSTALL_DATA) $(srcdir)/index.html $(kde_htmldir)/ru/kdevelop/index.html
	$(INSTALL_DATA) $(srcdir)/index-1.html $(kde_htmldir)/ru/kdevelop/index-1.html
	$(INSTALL_DATA) $(srcdir)/index-2.html $(kde_htmldir)/ru/kdevelop/index-2.html
	$(INSTALL_DATA) $(srcdir)/index-3.html $(kde_htmldir)/ru/kdevelop/index-3.html
	$(INSTALL_DATA) $(srcdir)/index-4.html $(kde_htmldir)/ru/kdevelop/index-4.html
	$(INSTALL_DATA) $(srcdir)/index-5.html $(kde_htmldir)/ru/kdevelop/index-5.html
	$(INSTALL_DATA) $(srcdir)/index-6.html $(kde_htmldir)/ru/kdevelop/index-6.html

uninstall-local:
	-rm -f $(kde_htmldir)/ru/kdevelop/index.html
	-rm -f $(kde_htmldir)/ru/kdevelop/index-1.html
	-rm -f $(kde_htmldir)/ru/kdevelop/index-2.html
	-rm -f $(kde_htmldir)/ru/kdevelop/index-3.html
	-rm -f $(kde_htmldir)/ru/kdevelop/index-4.html
	-rm -f $(kde_htmldir)/ru/kdevelop/index-5.html
	-rm -f $(kde_htmldir)/ru/kdevelop/index-6.html
