#!/usr/bin/perl -w
#
use strict;
use warnings;


#program version
my $VERSION="%{VERSION}";

#For CVS , use following line
#my $VERSION=sprintf("%d.%02d", q$Revision$ =~ /(\d+)\.(\d+)/);

sub do_something {
  my $text=shift;
  print "$text\n";
}

&do_something("Hello World");
print "That's all folks for version $VERSION \n";

__END__

=head1 NAME

%{APPNAME} - short discription of your program

=head1 SYNOPSIS

 how to us your program

=head1 DESCRIPTION

 long description of your program

=head1 SEE ALSO

 need to know things before somebody uses your program

=head1 AUTHOR

 %{AUTHOR}

=cut
