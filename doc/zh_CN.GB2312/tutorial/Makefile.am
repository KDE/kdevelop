####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = index-1.html index-2.html index-3.html index-4.html index.html index.sgml logotp3.png 

####### kdevelop will overwrite this part!!! (end)############
EXTRA_DIST = index.sgml logotp3.png \
            index.html index-1.html index-2.html index-3.html index-4.html

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/zh_CN.GB2312/kdevelop/tutorial
	$(INSTALL_DATA) $(srcdir)/logotp3.png $(kde_htmldir)/zh_CN.GB2312/kdevelop/tutorial/logotp3.png
	$(INSTALL_DATA) $(srcdir)/index.html $(kde_htmldir)/zh_CN.GB2312/kdevelop/tutorial/index.html
	$(INSTALL_DATA) $(srcdir)/index-1.html $(kde_htmldir)/zh_CN.GB2312/kdevelop/tutorial/index-1.html
	$(INSTALL_DATA) $(srcdir)/index-2.html $(kde_htmldir)/zh_CN.GB2312/kdevelop/tutorial/index-2.html
	$(INSTALL_DATA) $(srcdir)/index-3.html $(kde_htmldir)/zh_CN.GB2312/kdevelop/tutorial/index-3.html
	$(INSTALL_DATA) $(srcdir)/index-4.html $(kde_htmldir)/zh_CN.GB2312/kdevelop/tutorial/index-4.html

uninstall-local:
	-rm -f $(kde_htmldir)/zh_CN.GB2312/kdevelop/tutorial/logotp3.png
	-rm -f $(kde_htmldir)/zh_CN.GB2312/kdevelop/tutorial/index.html
	-rm -f $(kde_htmldir)/zh_CN.GB2312/kdevelop/tutorial/index-1.html
	-rm -f $(kde_htmldir)/zh_CN.GB2312/kdevelop/tutorial/index-2.html
	-rm -f $(kde_htmldir)/zh_CN.GB2312/kdevelop/tutorial/index-3.html
	-rm -f $(kde_htmldir)/zh_CN.GB2312/kdevelop/tutorial/index-4.html
