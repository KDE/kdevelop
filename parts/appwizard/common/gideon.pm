use File::Copy;
use Getopt::Long;
use IO::Handle;

sub customize
{
        my ( $filename ) = @_;

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


sub installDocbook()
{
        print "Installing Docbook template\n";
        mkdir( "${dest}/doc", 0777 );
        install( "${src}/template-common/kde-doc-Makefile.am", "${dest}/doc/Makefile.am" );
        mkdir( "${dest}/doc/en", 0777 );
        install( "${src}/template-common/kde-doc-en-Makefile.am", "${dest}/doc/en/Makefile.am" );
        install( "${src}/template-common/kde-index.docbook", "${dest}/doc/en/index.docbook" );
}

1;
