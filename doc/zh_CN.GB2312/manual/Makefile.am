####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = index.html index.sgml logotp3.png 

####### kdevelop will overwrite this part!!! (end)############
EXTRA_DIST = index.sgml index.html

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/zh_CN.GB2312/kdevelop
	$(INSTALL_DATA) $(srcdir)/index.html $(kde_htmldir)/zh_CN.GB2312/kdevelop/index.html

uninstall-local:
	-rm -f $(kde_htmldir)/zh_CN.GB2312/kdevelop/index.html
