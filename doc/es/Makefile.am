####### kdevelop will overwrite this part!!! (begin)##########

SUBDIRS = cref manual welcome 

EXTRA_DIST = es.po tip.database 

####### kdevelop will overwrite this part!!! (end)############
EXTRA_DIST = tip.database

SUBDIRS = welcome cref manual

install-data-local:
	$(mkinstalldirs) $(kde_htmldir)/es/kdevelop/
	$(INSTALL_DATA) $(srcdir)/tip.database $(kde_htmldir)/es/kdevelop/tip.database

uninstall-local:
	-rm -f $(kde_htmldir)/es/kdevelop/tip.database

