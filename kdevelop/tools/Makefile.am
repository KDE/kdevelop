####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = processes.pl processes.pl.in processesend.pl 

####### kdevelop will overwrite this part!!! (end)############
tools_DATA =  processes.pl processesend.pl 

toolsdir = $(kde_datadir)/kdevelop/tools/

EXTRA_DIST = $(tools_DATA)

processes.pl: processes.pl.in
	cp processes.pl.in processes.pl
	perl -npi -e 's%^\$$templatedir = _;$$%\$$templatedir = \"'${kde_datadir}'/kdevelop/templates\";%g;' \
	    processes.pl
	perl -npi -e 's%^\$$kde_icondir = _;$$%\$$kde_icondir = \"'${kde_icondir}'\";%g;' \
	    processes.pl
