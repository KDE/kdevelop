use File::Copy;
use Getopt::Long;
use IO::Handle;

sub customize
{
        my ( $filename ) = @_;
	my $year = ( localtime )[5] + 1900;

        open(IN, "<$filename") || die "Could not open for reading: $filename\n";
        open(OUT, ">$filename.tmp") || die "Could not open for writing: $filename.tmp\n";
        while (<IN>) {
              s/\$AUTHOR\$/$AUTHOR/g;
              s/\$EMAIL\$/$EMAIL/g;
              s/\$VERSION\$/$VERSION/g;
              s/\$APPNAMELC\$/$APPNAMELC/g;
              s/\$APPNAMEUC\$/$APPNAMEUC/g;
              s/\$APPNAME\$/$APPNAME/g;
              s/\$LICENSE\$/$LICENSE/g;
              s/\$LICENSEFILE\$/$LICENSEFILE/g;
              s/\$YEAR\$/$year/g;
              print OUT $_;
         }
         close IN;
         close OUT;
         move( "$filename.tmp", $filename );
}

sub customizeHTML
{
# HTML-escape strings for docbook stuff
	my $H_AUTHOR = $AUTHOR;
	my $H_EMAIL = $EMAIL;
	my $H_VERSION = $VERSION;
	my $H_APPNAMELC = $APPNAMELC;
	my $H_APPNAMEUC = $APPNAMEUC;
	my $H_APPNAME = $APPNAME;
	my $H_LICENSE = $LICENSE;
	my $H_LICENSEFILE = $LICENSEFILE;
        my $year = ( localtime )[5] + 1900;
	$H_AUTHOR =~ s/([^\w\s])/'&#'.ord($1).';'/ge;
	$H_EMAIL =~ s/([^\w\s])/'&#'.ord($1).';'/ge;
	$H_VERSION =~ s/([^\w\s])/'&#'.ord($1).';'/ge;
	$H_APPNAMELC =~ s/([^\w\s])/'&#'.ord($1).';'/ge;
	$H_APPNAMEUC =~ s/([^\w\s])/'&#'.ord($1).';'/ge;
	$H_APPNAME =~ s/([^\w\s])/'&#'.ord($1).';'/ge;
	$H_LICENSE =~ s/([^\w\s])/'&#'.ord($1).';'/ge;
	$H_LICENSEFILE =~ s/([^\w\s])/'&#'.ord($1).';'/ge;

        my ( $filename ) = @_;

        open(IN, "<$filename") || die "Could not open for reading: $filename\n";
        open(OUT, ">$filename.tmp") || die "Could not open for writing: $filename.tmp\n";
        while (<IN>) {
              s/\$AUTHOR\$/$H_AUTHOR/g;
              s/\$EMAIL\$/$H_EMAIL/g;
              s/\$VERSION\$/$H_VERSION/g;
              s/\$APPNAMELC\$/$H_APPNAMELC/g;
              s/\$APPNAMEUC\$/$H_APPNAMEUC/g;
              s/\$APPNAME\$/$H_APPNAME/g;
              s/\$LICENSE\$/$H_LICENSE/g;
              s/\$LICENSEFILE\$/$H_LICENSEFILE/g;
              s/\$YEAR\$/$year/g;
              print OUT $_;
         }
         close IN;
         close OUT;
         move( "$filename.tmp", $filename );	
}

sub install
{
        my ( $srcfilename, $destfilename ) = @_;

        copy( $srcfilename, $destfilename ) 
          || die "Could not copy $srcfilename to $destfilename: $!";
        customize( $destfilename );
}

sub installHTML
{
        my ( $srcfilename, $destfilename ) = @_;

        copy( $srcfilename, $destfilename )
          || die "Could not copy $srcfilename to $destfilename: $!";
        customizeHTML( $destfilename );
}

sub initGideon
{
        autoflush STDOUT 1;

        GetOptions( 'author=s'        => \$AUTHOR,
                    'email=s'         => \$EMAIL,
                    'version=s'       => \$VERSION,
                    'source=s'        => \$src,
                    'dest=s'          => \$dest,
                    'appname=s'       => \$APPNAME,
                    'filetemplates:s' => \$FILETEMPLATES,
                    'license=s'       => \$LICENSE,
                    'licensefile:s'   => \$LICENSEFILE ) || die "Wrong options\n";

        $APPNAMELC = lc $APPNAME;
        $APPNAMEUC = uc $APPNAME;

        print "Making destination directory\n";
        mkdir( "${dest}", 0777 );
}


sub installFileTemplate
{
        return if $FILETEMPLATES eq "";

        print "Installing file templates\n";
        mkdir( "${dest}/templates", 0777 );
        my %templates = split(/,/, $FILETEMPLATES);
        while (($suffix, $filename) = each %templates) {
                install( $filename, "${dest}/templates/$suffix" );
        }
}


sub installLicense
{
        return if $LICENSEFILE eq "";

        print "Installing license file\n";
        install( "${src}/template-common/$LICENSEFILE", "${dest}/$LICENSEFILE" );
}


sub installAdmin
{
        print "Installing admin directory\n";
        copy( "${src}/template-common/admin.tar.gz", "${dest}/admin.tar.gz" );
        chdir( $dest ) || die "Could not chdir to $dest\n";
        system( 'gunzip', '-f', 'admin.tar.gz' );
        system( 'tar', 'xf', 'admin.tar' );
        unlink( "admin.tar" );
}


sub installIncAdmin
{
        print "Installing automake administrative stuff\n";
        copy( "${src}/template-common/incadmin.tar.gz", "${dest}/incadmin.tar.gz" );
        chdir( $dest ) || die "Could not chdir to $dest\n";
        system( 'gunzip', '-f', 'incadmin.tar.gz' );
        system( 'tar', 'xf', 'incadmin.tar' );
        unlink( "incadmin.tar" );
}


sub installGNU()
{
        print "Installing GNU coding standard files\n";
        copy( "${src}/template-common/gnu.tar.gz", "${dest}/gnu.tar.gz" );
        chdir( $dest ) || die "Could not chdir to $dest\n";
        system( 'gunzip', '-f', 'gnu.tar.gz' );
        system( 'tar', 'xf', 'gnu.tar' );
        unlink( "gnu.tar" );
        customize( "${dest}/AUTHORS" );
        customize( "${dest}/COPYING" );
        customize( "${dest}/ChangeLog" );
        customize( "${dest}/INSTALL" );
        customize( "${dest}/README" );
        customize( "${dest}/TODO" );
}


sub installGNOME
{
        print "Installing GNOME autoconf macros\n";
        copy( "${src}/template-common/gnome.tar.gz", "${dest}/gnome.tar.gz" );
        chdir( $dest ) || die "Could not chdir to $dest\n";
        system( 'gunzip', '-f', 'gnome.tar.gz' );
        system( 'tar', 'xf', 'gnome.tar' );
        move( "macros/gnome-Makefile.am", "macros/Makefile.am" );
        unlink( "gnome.tar" );
}

sub installWX
{
        print "Installing wxWindows autoconf macros\n";
		  copy( "${src}/template-common/wxwindows.tar.gz", "${dest}/wxwindows.tar.gz" );
		  chdir( $dest ) || die "Could not chdir to $dest\n";
        system( 'gunzip', '-f', 'wxwindows.tar.gz' );
        system( 'tar', 'xf', 'wxwindows.tar' );
        unlink( "wxwindows.tar" );
}

sub installDocbook()
{
        print "Installing Docbook template\n";
        mkdir( "${dest}/doc", 0777 );
        install( "${src}/template-common/kde-doc-Makefile.am", "${dest}/doc/Makefile.am" );
        mkdir( "${dest}/doc/en", 0777 );
        install( "${src}/template-common/kde-doc-en-Makefile.am", "${dest}/doc/en/Makefile.am" );
        installHTML( "${src}/template-common/kde-index.docbook", "${dest}/doc/en/index.docbook" );
}

1;
