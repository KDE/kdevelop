#!/usr/bin/perl -w
require ("flush.pl");
use File::Copy;

$homedirectory = $ENV{HOME};
printflush (STDOUT,"Starting with installation\n");

#open file "entries" for reading the parameters from kAppWizard and put it in a hash
open (PROCESSLIST,$homedirectory . "/.kde/share/apps/kdevelop/entries") || die "kann Datei nicht öffnen: $!";
while ( defined ($name = <PROCESSLIST> )) {
    chomp ($name);
    $process = <PROCESSLIST>;
    chomp ($process);
    $processes{$name} = $process;
}
close (PROCESSLIST);

$nameLittle = $processes{NAME};
$nameLittle =~ tr/A-Z/a-z/;
$nameBig = $processes{NAME};
$nameBig =~ tr/a-z/A-Z/;
$name =  $processes{NAME};

$overDirectory = $processes{DIRECTORY} . "/" . $processes{PRJVSLOCATION} . "/";
$underDirectory = $overDirectory . $nameLittle;

#if ($processes{VSSUPPORT} eq "cvs") {
#  	chdir ($homedirectory . "/kdeveloptemp/");
#  	$vsimport = "cvs -d " . $processes{VSLOCATION} . " import -m \"" . $processes{LOGMESSAGE} . "\" " .
#    						$processes{PRJVSLOCATION} . " "  . $processes{VENDORTAG} . " " . $processes{RELEASETAG};
#    system ("$vsimport");
#}

#if ($processes{VSSUPPORT} ne "none") {
#		chdir ($homedirectory);
#		$removetemp ="rm -r -f " . $homedirectory . "/kdeveloptemp";
# 	 	system ("$removetemp");

#  	chdir ($processes{DIRECTORY});
#		$checkout = "cvs -d " . $processes{VSLOCATION} . " co " . $processes{PRJVSLOCATION};
#		system ("$checkout");
#  }

if ($processes{APPLICATION} eq "customproj") {}
else {
  #start make -f Makefile.dist
  printflush (STDOUT,"create configurationfiles...\n");
  chdir ($overDirectory);
  system ("make -f Makefile.dist");
  
  #start configure
  printflush (STDOUT,"make configure...\n");
  chdir ($overDirectory);
  system ("CFLAGS=\"-O0 -g3 -Wall\" CXXFLAGS=\"-O0 -g3 -Wall\" ./configure");
}
#if User-Documentation was chosen in kAppWizard
if ($processes{USER} eq "yes") {
  
  #create the html-files
  printflush (STDOUT,"configure files...\n");
  chdir ($underDirectory . "/docs/en");
  if (-e "index.nif") {
  	system ("ksgml2html index.sgml en");
  }
  else {
  	system ("sgml2html index.sgml");
  }
}

chdir ($underDirectory);
printflush (STDOUT,"READY\n");

exit;
















