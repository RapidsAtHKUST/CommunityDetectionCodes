#!/usr/local/bin/perl

use strict;
use Getopt::Long;

my $cmode = 'extend';
my $rmode = 'extend';
my $etcai = 0;

if
(! GetOptions
   (  "cmode=s"   =>    \$cmode
   ,  "rmode=s"   =>    \$rmode
   ,  "etcai"     =>    \$etcai
   )
)
   {  print STDERR "option processing failed\n";
      exit(1);
   }

my $cstrict = $cmode eq 'restrict';
my $rstrict = $rmode eq 'restrict';
my $ai = 0;

while (<>) {
   chomp;
   my @F = split;
   my $l = $etcai ? $ai : shift @F;

   next if $cstrict && !$etcai && lc($l) ne $l;

   my @f = grep { !$rstrict || lc($_) eq $_ } @F;
   next if $etcai && !@f;

   $ai++;

   for (@f) {
      print "$l\t$_\n";
   }
}

