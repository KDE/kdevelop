####### kdevelop will overwrite this part!!! (begin)##########

SUBDIRS = welcome 

EXTRA_DIST = tip.database 

####### kdevelop will overwrite this part!!! (end)############
EXTRA_DIST =tip.database

SUBDIRS = welcome

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/nl/kdevelop/
	$(INSTALL_DATA) $(srcdir)/tip.database $(kde_htmldir)/nl/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/nl/kdevelop/tip.database
