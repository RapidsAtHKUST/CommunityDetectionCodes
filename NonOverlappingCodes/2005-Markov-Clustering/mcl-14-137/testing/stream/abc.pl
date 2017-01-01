#!/usr/local/bin/perl -w

use strict;
my ($N, $k, $sd) = @ARGV;

$N = 100 unless defined($N);
$k = 1+int(($N/10)) unless defined $k;
$sd = $k-1 unless defined $sd;

my @w = map { chr($_) } (ord('a')..ord('z'), ord('A')..ord('Z'));


while ($N-- > 0) {
   my $d = $sd - int(rand(2*$sd));
   my $l = $k + $d;

   print $w[int(rand(@w))];
   while ($l-- > 0) {
      print "\t", $w[int(rand(@w))];
   }
   print "\n";
}



