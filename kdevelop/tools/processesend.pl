#!/usr/bin/perl -w
require ("flush.pl");
use File::Copy;

$homedirectory = $ENV{HOME};
printflush (STDOUT,"Starting with configuration\n");

#open file "entries" for reading the parameters from kAppWizard and put it in a hash
open (PROCESSLIST,$homedirectory . "/.kde/share/apps/kdevelop/entries") 
  || die "can not open file \"entries\" in your local app-data-dir:\n  $!";
while ( defined ($name = <PROCESSLIST> )) {
  chomp ($name);
  chomp ($process = <PROCESSLIST>);
  $processes{$name} = $process;
}
close (PROCESSLIST);

$nameLittle = $processes{NAME};
$nameLittle =~ tr/A-Z/a-z/;
$nameBig = $processes{NAME};
$nameBig =~ tr/a-z/A-Z/;
$name =  $processes{NAME};

$overDirectory = $processes{DIRECTORY} . "/";
$underDirectory = $overDirectory . $nameLittle;

if ($processes{APPLICATION} eq "customproj") {}
else {
  #start make -f Makefile.dist
  printflush (STDOUT,"create configuration files...\n");
  chdir ($overDirectory);
  system ("make -f Makefile.dist");
  
  #start configure
  printflush (STDOUT,"make configure...\n");
  chdir ($overDirectory);
	if ($processes{APPLICATION} eq "qt2normal") {
	  system ("LDFLAGS=\" \" CFLAGS=\"-O0 -g3 -Wall\" CXXFLAGS=\"-O0 -g3 -Wall\" ./configure $processes{CONFIGARG}");
	
	}
	else{
  	system ("LDFLAGS=\" \" CFLAGS=\"-O0 -g3 -Wall\" CXXFLAGS=\"-O0 -g3 -Wall\" ./configure");
	}
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
