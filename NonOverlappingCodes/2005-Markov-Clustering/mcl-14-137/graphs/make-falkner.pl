#!/usr/bin/perl -w

use Getopt::Long;
use strict;

my @ARGV_COPY  = @ARGV;
my $n_args = @ARGV;

$::debug =  0;
$::test  =  0;
my $help  =  0;
my $foo   = '';
my $fndata = "";
my $fndefs = "";
my $progname = 'funkelectric';
my $R      =  2.0;
my $radius =  0.34;

# --debug                 debug
# --test                  test

sub help {
   print <<EOH;
Usage:
   $progname [options]
Options:
--help                  this
--data=<fname>          data in <x> <y> format, id 0, 1, 2, ..
--mink=<f>              minkovsky distance parameter
--radius=<f>            cutoff for inclusion (note: inverted distance)
--defs=<fname>          write postscript definitions "/v0[1 4]def" etc
EOH
}

if
(! GetOptions
   (  "help"            =>   \$help
   ,  "test"            =>   \$::test
   ,  "debug=i"         =>   \$::debug
   ,  "data=s"          =>   \$fndata
   ,  "defs=s"          =>   \$fndefs
   ,  "mink=f"          =>   \$R
   ,  "radius=f"        =>   \$radius
   )
)
   {  print STDERR "option processing failed\n";
      exit(1);
   }

if ($help) {
   help();
   exit(0);
}


my $fh = \*DATA;
if ($fndata) {
   if ($fndata eq '-') {
      $fh = \*STDIN;
   }
   else {
      open(FH, "<$fndata") || die "no can do no open file named $fndata";
      $fh = \*FH;
   }
}

my %co = ();

while (<$fh>) {
   chomp;
   next if /^\s*#/;
   next unless /\S+\s*\S+/;
   my ($c, $x, $y) = split;
   $co{$c} = [$x, $y];
}

my $dim = keys %co;

if ($fndefs) {
   open (DEFS, ">$fndefs") || die "cannot open $fndefs for writing";
   print DEFS "\n\n";
   for my $c (sort {$a <=> $b} keys %co) {
      print DEFS "/v$c\[$co{$c}[0] $co{$c}[1]]def\n";
   }
   print DEFS "\n\n";
   close DEFS;
}

my $dims = $dim . 'x' . $dim;
print <<EOH;
(mclheader
mcltype matrix
dimensions $dims
)
(mclmatrix
begin
EOH

for my $c (sort { $a <=> $b } keys %co) {
   my ($u, $v) = @{$co{$c}};
   printf "%-4d", $c;
   for my $d (sort { $a <=> $b } keys %co) {
      next if $c == $d;
      my ($x, $y) = @{$co{$d}};
      my $d1 = $x-$u;
      my $d2 = $y-$v;
      $d1 *= -1 if $d1 < 0;
      $d2 *= -1 if $d2 < 0;
      my $sim = 1/(($d1 ** $R + $d2 ** $R)**(1/$R));
# print ".. [$c($u,$v) $d($x,$y)] [$d1 $d2] [$R $sim $radius]\n";
# next;
      next if 1/sqrt($d1**2 + $d2**2) < $radius;
      printf " $d:%.3f", $sim;
   }
   print " \$\n";
}

print ")\n";

__DATA__
0 4 1
1 5 1
2 7 1
3 8 1
4 10 1
5 11 1
6 16 1
7 17 1
8 18 1
9 3 2
10 4 2
11 6 2
12 7 2
13 11 2
14 15 2
15 16 2
16 17 2
17 4 3
18 5 3
19 6 3
20 7 3
21 8 3
22 10 3
23 14 3
24 15 3
25 17 3
26 18 3
27 19 3
28 1 4
29 2 4
30 10 4
31 12 4
32 13 4
33 15 4
34 16 4
35 18 4
36 19 4
37 4 5
38 5 5
39 8 5
40 9 5
41 11 5
42 12 5
43 3 6
44 7 6
45 9 6
46 10 6
47 12 6
48 13 6
49 15 6
50 18 6
51 1 7
52 4 7
53 12 7
54 13 7
55 14 7
56 16 7
57 17 7
58 18 7
59 19 7
60 3 8
61 4 8
62 5 8
63 6 8
64 7 8
65 14 8
66 15 8
67 19 8
68 3 9
69 5 9
70 9 9
71 11 9
72 14 9
73 15 9
74 16 9
75 19 9
76 1 10
77 2 10
78 5 10
79 14 10
80 15 10
81 17 10
82 19 10
83 3 11
84 6 11
85 7 11
86 10 11
87 13 11
88 15 11
89 18 11
90 1 12
91 2 12
92 3 12
93 7 12
94 9 12
95 10 12
96 15 12
97 16 12
98 18 12
99 2 13
100 3 13
101 4 13
102 6 13
103 12 13
104 13 13
105 17 13
106 4 14
107 5 14
108 10 14
109 13 14
110 16 14
111 17 14
112 1 15
113 2 15
114 3 15
115 6 15
116 14 15
117 17 15
118 2 16
119 4 16
120 5 16
121 8 16
122 12 16
123 13 16
124 14 16
125 17 16
126 18 16
127 19 16
128 1 17
129 4 17
130 7 17
131 8 17
132 9 17
133 10 17
134 12 17
135 14 17
136 16 17
137 5 18
138 9 18
139 11 18
140 12 18
141 13 18
142 19 18
143 1 19
144 2 19
145 5 19
146 7 19
147 9 19
148 13 19
149 14 19
