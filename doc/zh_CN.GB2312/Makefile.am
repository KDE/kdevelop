EXTRA_DIST = tip.database
SUBDIRS = welcome tutorial manual

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/zh_CN.GB2312/kdevelop/
	$(INSTALL_DATA) $(srcdir)/tip.database $(kde_htmldir)/zh_CN.GB2312/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/zh_CN.GB2312/kdevelop/tip.database
