####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = index.html 

####### kdevelop will overwrite this part!!! (end)############

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/en/kdevelop/reference/C/
	$(INSTALL_DATA) $(srcdir)/index.html $(kde_htmldir)/en/kdevelop/reference/C/index.html

uninstall-local:
	-rm -f $(kde_htmldir)/en/kdevelop/reference/C/index.html
