EXTRA_DIST = tip.database

SUBDIRS = manual tutorial welcome

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/zh_CN.GB2312/kdevelop/
	$(INSTALL_DATA) tip.database $(kde_htmldir)/zh_CN.GB2312/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/zh_CN.GB2312/kdevelop/tip.database
