####### kdevelop will overwrite this part!!! (begin)##########

SUBDIRS = manual tutorial welcome 

EXTRA_DIST = tip.database 

####### kdevelop will overwrite this part!!! (end)############
EXTRA_DIST = tip.database
SUBDIRS = welcome tutorial manual

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/zh_CN.GB2312/kdevelop/
	$(INSTALL_DATA) $(srcdir)/tip.database $(kde_htmldir)/zh_CN.GB2312/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/zh_CN.GB2312/kdevelop/tip.database
