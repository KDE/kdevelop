####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = index.html 

####### kdevelop will overwrite this part!!! (end)############
EXTRA_DIST =index.html

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/es/kdevelop/reference/C/
	$(INSTALL_DATA) $(srcdir)/index.html $(kde_htmldir)/es/kdevelop/reference/C/index.html

uninstall-local:
	-rm -f $(kde_htmldir)/es/kdevelop/reference/C/index.html
