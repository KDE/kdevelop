####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = kdevdlg2ui kimport one2two processesend.pl processes.pl.in 

####### kdevelop will overwrite this part!!! (end)############


tools_DATA =  processesend.pl processes.pl kimport

bin_SCRIPTS = kdevdlg2ui one2two

toolsdir = $(kde_datadir)/kdevelop/tools

processes.pl: processes.pl.in
	cp $(srcdir)/processes.pl.in processes.pl
	perl -npi -e 's%^\$$templatedir = _;$$%\$$templatedir = \"'${kde_datadir}'/kdevelop/templates\";%g;' \
	    processes.pl
	perl -npi -e 's%^\$$kde_icondir = _;$$%\$$kde_icondir = \"'${kde_icondir}'\";%g;' \
	    processes.pl
