#!/usr/local/bin/perl -w

use strict;
use Data::Dumper;
use Getopt::Long;

my %seen_as_single = ();

my %top = ();              # track the tops of all trees.

my %top_to_leaf = ();
my %leaf_to_top = ();

my $landscape = 0;
my $debug = 0;
my $cutoff = undef;
my $help = 0;


my $int = 1;
my $i = 0;
my $max_height = 0;
my $max_tree_size = 0;

my @times = times();
my @n_args = @ARGV;


sub help {
   print <<EOH;
Usage:
   tree3.pl [options]
Options:
--landscape             produce landscape output
--cutoff=f              produce thresholded SL clustering
--debug                 debug
EOH
}

if
(! GetOptions
   (  "help"            =>   \$help
   ,  "debug=i"         =>   \$debug
   ,  "cutoff=f"        =>   \$cutoff
   ,  "landscape"       =>   \$landscape
   )
)
   {  print STDERR "option processing failed\n";
      exit(1);
   }

if ($help) {
   help();
}


my $prev_val = 1.0;

while (<>) {

   my ($x, $y, $val) = ("", "", 0);

   if (/([^\t]+)\t([^\t]+)\t(\S+)/) {
      ($x, $y, $val) = ($1, $2, +$3);
   }
   elsif (/(\S+)\s+(\S+)\s+(\S+)/) {
      ($x, $y, $val) = ($1, $2, +$3);
   }
   elsif (/(\S+)\s+(\S+)/) {
      ($x, $y, $val) = ($1, $2, $prev_val);
   }
   else {
      next;
   }

   $prev_val = $val;

   print STDERR '.' if ++$i % 1000 == 0;
   if ($i % 40000 == 0) {
      my @times2 = times();
      my $udiff = $times2[0] - $times[0];
      my $sdiff = $times2[1] - $times[1];
      printf STDERR " %10d sz:%5d ht:%5d utime: %.2f stime: %.2f\n", $i, $max_tree_size, $max_height, $udiff, $sdiff;
      @times = @times2;
   }

   if ($x eq $y) {
      $seen_as_single{$x} = 1;
      next;
   }

   my ($topx, $topy);

   if (defined($leaf_to_top{$x})) {
      $topx = $leaf_to_top{$x};
   }
   else {
      $topx = { name => $x, size => 1, branch_level => 1, height => 1 };
      $top_to_leaf{$topx->{name}} = [$x];
   }
   if (defined($leaf_to_top{$y})) {
      $topy = $leaf_to_top{$y};
   }
   else {
      $topy = { name => $y, size => 1, branch_level => 1, height => 1 };
      $top_to_leaf{$topy->{name}} = [$y];
   }

   next if $topx->{name} eq $topy->{name}; 
   my $sizexy = $topx->{size} + $topy->{size};

# print STDERR "joining trees $topx->{name} and $topy->{name}\n";

   my ($left, $right, $name_left, $name_right) = ($topx, $topy, $x, $y);

   if ($topy->{size} > $topx->{size}) {
      ($left, $right, $name_left, $name_right) = ($topy, $topx, $y, $x);
   }

   my $topxy =
   {  name =>  "__INT__[$int]"
   ,  size =>  $sizexy
   ,  left =>  $left
   ,  right => $right
   ,  left_name => $name_left
   ,  right_name => $name_right
   ,  branch_level => $val
   }  ;

   $topxy->{height} = $topx->{height}+1;
   $topxy->{height} = $topy->{height}+1 if $topy->{height} > $topx->{height};

   $max_height = $topxy->{height} if $topxy->{height} > $max_height;
   $max_tree_size = $sizexy if $sizexy > $max_tree_size;

# print STDERR "$x [$topx->{name}] $y [$topy->{name}] ---> $topxy->{name}\n";

   my $all_leaves_x = $top_to_leaf{$topx->{name}};
   my $all_leaves_y = $top_to_leaf{$topy->{name}};

#  $all_leaves_x = [$x] if !defined($all_leaves_x);
#  $all_leaves_y = [$y] if !defined($all_leaves_y);
#        #
#        #  ^ This exception case is a bit worrisome.

   @leaf_to_top{@$all_leaves_x} = ($topxy) x @$all_leaves_x;
   @leaf_to_top{@$all_leaves_y} = ($topxy) x @$all_leaves_y;

   $int++;

   delete($top_to_leaf{$topy->{name}});
   delete($top_to_leaf{$topx->{name}});

   $top_to_leaf{$topxy->{name}} = [@$all_leaves_x, @$all_leaves_y];

   delete($top{$topy->{name}});
   delete($top{$topx->{name}});
   $top{$topxy->{name}} = $topxy;

   local $, = ' ';
# print STDERR "KEYS", keys %top, "\n";
}


sub peak_tree {
   my ($node) = (@_);
   if ($node->{size} == 1) {
      print "$node->{name}\n";
   }
   else {
      peak_tree($node->{left});
      print "$node->{branch_level}\n";
      peak_tree($node->{right});
   }
}


sub tree_landscape_dump {

   my ($root) = (@_);
   my @list = ({ NODE => $root });
   my $i = 0;

   while (@list) {
      my $thingy = pop @list;

      if (defined($thingy->{VALUE})) {
         print "$thingy->{VALUE}\n";
         next;
      }
      my $node = $thingy->{NODE};

      if ($node->{size} == 1) {
         print "$node->{name}\n";
      }
      else {
         push @list, { NODE => $node->{right} };
         push @list, { VALUE => $node->{branch_level} };
         push @list, { NODE => $node->{left} };
      }
   }
}


sub join_all_labels {
   my @in = @_;
   my $out = "";

   while (my $node = pop @in) {
      if ($node->{size} == 1) {
         $out .= $node->{name} . "\t";
      }
      else {
         push @in, ($node->{left}, $node->{right});
      }
   }
   chop $out;
   return $out;
}


sub print_flat_clustering {

   my ($root, $cutoff) = (@_);
   my @in = ( $root );

   while (@in) {

      my $node = pop @in;

      if (!defined($node->{branch_level})) {
         print keys %$node;
         die "most unexpectedly";
      }
      elsif ($node->{branch_level} >= $cutoff) {
         print join_all_labels($node), "\n";
      }
      else {
         push @in, $node->{left}, $node->{right};
      }
   }
}



sub tree_stringify {

   my ($root) = (@_);
   my @in = ( { NODE => $root } );
   my $out = "";

   while (@in) {

      my $thingy = pop @in;

      if (defined($thingy->{VALUE})) {
         $out .= "$thingy->{VALUE}";
         next;
      }

      my $node = $thingy->{NODE};

      if ($node->{size} == 1) {
         $out .= "$node->{name}:0";
      }
      else {
         my $left    =  $node->{left};
         my $right   =  $node->{right};
         my $d1      =  $node->{right}{branch_level} - $node->{branch_level};
         my $d2      =  $node->{left}{branch_level} - $node->{branch_level};
         my $d3      =  $d1 < $d2 ? $d1 : $d2;
         my $val     =  sprintf "%.10f", $d3;

         $out .= "(";

         push @in,
            (  { VALUE  =>    "):$val" }
            ,  { NODE   =>    $right   }
            ,  { VALUE  =>    ','      }
            ,  { NODE   =>    $left    }
            )
      }
   }

   return "$out";
}



my @tops = sort { $top{$b}{size} <=> $top{$a}{size} } keys %top;
my $sgl = join ',',
            map { "$_:0" }
               grep { !defined($leaf_to_top{$_}) }
                  keys %seen_as_single;

{  local $" = ',';
   # print STDERR "@tops\n";
}


if ($landscape) {
   my $tree = join ',', map { tree_stringify($top{$_}) } @tops;

   if ($sgl) {
      print "($tree,$sgl)\n";
   }
   elsif (@tops > 1) {
      print "$tree\n";
   }
   else {
      print "($tree)\n";
   }

   my $t_first = shift @tops;

   tree_landscape_dump($top{$t_first});

   for my $t (@tops) {
      print "0\n";
      tree_landscape_dump($top{$t});
   }

   for my $s (grep { !defined($leaf_to_top{$_}) } sort keys %seen_as_single) {
      print "0\n$s\n";
   }
}


elsif (defined($cutoff)) {
   for my $t (@tops) {
      print_flat_clustering($top{$t}, $cutoff)
      # print Dumper($top{$t});
   }
   for my $s (grep { !defined($leaf_to_top{$_}) } sort keys %seen_as_single) {
      print "$s\n";
   }
}




##  my $t_first = shift @tops;
##  peak_tree($top{$t_first});
##  
##  for my $t (@tops) {
##     print "0\n";
##     peak_tree($top{$t});
##  }
##  
##  for my $s (grep { !defined($jump{$_}) } sort keys %seen_as_single) {
##     print "0\n$s\n";
##  }

