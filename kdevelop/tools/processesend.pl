#!/usr/bin/perl -w
require ("flush.pl");
use File::Copy;

$homedirectory = $ENV{KDEHOME};
$homedirectory = "$ENV{HOME}/.kde"    if (!$homedirectory);

my $entriesfilename= shift || "entries";
printflush (STDOUT,"Starting with configuration\n");

#open file "entries" for reading the parameters from kAppWizard and put it in a hash
open (PROCESSLIST,$homedirectory . "/share/apps/kdevelop/" . $entriesfilename)
  || die "can not open file \"$entriesfilename\" in your local app-data-dir:\n  $!";
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
  my $confargs = $processes{CONFIGARG} || "";
  #start make -f Makefile.dist
  printflush (STDOUT,"creating configuration files...\n");
  printflush (STDOUT,">make -f Makefile.dist\n");
  chdir ($overDirectory);
  system ("make -f Makefile.dist");
  
  #start configure
  printflush (STDOUT,"make configure...\n");
  printflush (STDOUT,">LDFLAGS=\" \" CFLAGS=\"-O0 -g3 -Wall\" CXXFLAGS=\"-O0 -g3 -Wall\"\n>./configure $confargs\n");
  chdir ($overDirectory);
  system ("LDFLAGS=\" \" CFLAGS=\"-O0 -g3 -Wall\" CXXFLAGS=\"-O0 -g3 -Wall\" ./configure " . $confargs);
	
}

#if API-Files was chosen in kAppWizard
if ($processes{API} eq "yes")
{
	
  printflush (STDOUT,"creating API documentation...\n");
  chdir ($underDirectory);
  #create the API-documentation
      if (<*.h>)
      {
       if ($processes{KDOC_CALL})
       {
         my $call=$processes{KDOC_CALL};
         $call =~ s/\|UNDERDIRECTORY\|/$underDirectory/eg;
         printflush (STDOUT,">$call\n");

	 system ("$call");
       }
       else
       {
         printflush (STDOUT,">kdoc -d $underDirectory/api/ $name *.h\n");
	 system ("kdoc -d $underDirectory/api/ $name *.h");
       }
      }
      else
      {
        printflush (STDOUT,"No header files found, so no API-doc generation yet\n");
      }
}


#if User-Documentation was chosen in kAppWizard
if ($processes{USER} eq "yes") {
  
  #create the html-files
  chdir ($underDirectory . "/docs/en");

  if ( 	$processes{APPLICATION} eq "kde2normal" || $processes{APPLICATION} eq "kde2mini" ||
       	$processes{APPLICATION} eq "kde2mdi")
  {
     # FIXME: there should be the proper call for docbooks
  }
  else
  {
    printflush (STDOUT,"creating user documentation...\n");
    if (-e "index.nif")
    {
      printflush (STDOUT,">ksgml2html index.sgml en\n");
      system ("ksgml2html index.sgml en");
    }
    else
    {
      printflush (STDOUT,">sgml2html index.sgml\n");
      system ("sgml2html index.sgml");
    }
  }
}

chdir ($underDirectory);

if ($processes{APPLICATION} eq "kdemini" || $processes{APPLICATION} eq "kdenormal" ||
    $processes{APPLICATION} eq "kdenormalogl" )
  {
    printflush (STDOUT,"creating $nameLittle.pot\n");
    if ($processes{XGETTEXT} && $processes{XGETTEXT} eq "yes")
    {
      printflush (STDOUT,">make messages\n");
      system ("make messages >/dev/null");
    }
    else
    {
      printflush (STDOUT,"NO $nameLittle.pot creation without xgettext\n");
    }
  }

if ($processes{APPLICATION} eq "kde2normal" || $processes{APPLICATION} eq "kde2mini" ||
    $processes{APPLICATION} eq "kde2mdi")
  {
    printflush (STDOUT,"creating $nameLittle.pot\n");
    if ($processes{XGETTEXT} && $processes{XGETTEXT} eq "yes")
    {
      chdir ($overDirectory);
      printflush (STDOUT,">make messages\n");
      system ("make package-messages >/dev/null");
    }
    else
    {
      printflush (STDOUT,"NO $nameLittle.pot creation without xgettext\n");
    }
  }



printflush (STDOUT,"READY\n");

system("rm -f ". $homedirectory . "/.kde/share/apps/kdevelop/" . $entriesfilename);
exit;
