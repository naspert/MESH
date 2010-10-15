#! /usr/bin/perl
# $Id$

unless ($#ARGV >= 0) {
  die("Usage: raw2smf.pl infile[.raw] [outfile.smf]\n");
}


$infile = $ARGV[0];
if ($#ARGV == 0) {
  $outfile = $infile;
  unless ($infile =~ /.raw$/) {
    $infile = $infile.".raw";
    $outfile = $outfile.".smf";
  } else {
    $outfile =~ s/raw$/smf/;
  }
} else {
  $outfile = $ARGV[1];
  unless ($infile =~ /.raw$/) {
    $infile = $infile.".raw";
  }
  unless ($outfile =~/.smf$/) {
    $outfile = $outfile.".smf";
  }
}


open(INFILE, $infile) || die("Unable to open input file $infile\n");
open(OUTFILE, ">$outfile") || die("Unable to open output file $outfile\n");
#read 1st line
$_ = <INFILE>;
chomp;
@tmp = split(/\ /);
$vnum = $tmp[0];
$fnum = $tmp[1];
print OUTFILE "# \$vertices $vnum\n";
print OUTFILE "# \$faces $fnum\n";
$i = 0;
while ($i < $vnum) {
  $_ = <INFILE>;
  chomp;
  print OUTFILE "v $_\n";
  $i++;
}
$i = 0;
while ($i < $fnum) {
  $_ = <INFILE>;
  chomp;
  @tmp=split(/\ /);
  $tmp[0]++;
  $tmp[1]++;
  $tmp[2]++;
  print OUTFILE "f $tmp[0] $tmp[1] $tmp[2]\n";
  $i++;
}

close(INFILE);
close(OUTFILE);

