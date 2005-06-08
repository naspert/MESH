#! /usr/bin/perl -w
# $Id$

unless ($#ARGV == 0) {
  die ("Usage: vtk2raw.pl infile[.vtk]\n");
}

$filename = $ARGV[0];
if($filename !~ /\.vtk$/) {
  $filename = $filename.".vtk";
}
$outfile = $filename;
$outfile =~ s/\.vtk$/\.raw/;

#print "$filename $outfile\n";
open(INFILE,$filename);
open(OUTFILE,"> $outfile");

# Read all the vertices
do {
  $_ = <INFILE>;
} while ($_ !~ /POINTS/);

@tmp = split(/\ /);
$h_numvert = $tmp[1];
print "Header num_vert = $h_numvert\n";
$r_numvert = 0;
$_ = <INFILE>;
while ($_ =~ /^(-|[0-9])/) {
  chomp;
  @tmp = split(/\ /);
  $nvert = ($#tmp+1)/3;
#  print "#tmp=$#tmp nvert=$nvert\n";
  $r_numvert += $nvert;
  $i = 0;
  while($i < $nvert) {
    print OUTFILE "$tmp[3*$i] $tmp[3*$i+1] $tmp[3*$i+2]\n";
    $i++;
  }
  $_=  <INFILE>;
} 
print "Read $r_numvert vertices\n";

#Now read the faces
if ($_ =~ /^POLYGONS/) {
  @tmp = split(/\ /);
  $h_numfaces = $tmp[1];
  print "Header num_faces = $h_numfaces\n";
  $r_numfaces = 0;
  $_ = <INFILE>;
  while ($_ =~ /[3-9]/) {
    chomp;
    @tmp = split(/\ /);
    if ($tmp[0] != '3') {
      print "Warning : non-triangles found\n";
    }
    $i = 1;
    while ($i <= $#tmp) {
      print OUTFILE "$tmp[$i] ";
      $i++;
    }
    $r_numfaces++;
    print OUTFILE "\n";
    $_=<INFILE>;
  }
} else {
  die("POLYGONS field is not after POINTS\n");
}
print "Read $r_numfaces faces\n";

# And finally the normals
$_ = <INFILE>;
while ($_ !~ /^NORMALS/) {
  $_ = <INFILE>;
}
#print $_;
$r_normals = 0;

#chomp;
$_ = <INFILE>;
print $_;
while (defined($_) && ( /^(-|[0-9])/)) {
  chomp($_);
  @tmp = split(/\ /);
  $nnorm = ($#tmp+1)/3;
#  print "nnorm = $nnorm\n";
  $i = 0;
  while ($i < $nnorm) {
    print OUTFILE "$tmp[3*$i] $tmp[3*$i+1] $tmp[3*$i+2]\n";
    $r_normals++;
    $i++;
  }
  $_ = <INFILE>;
#print $_;
}
print "$r_normals normals read\n";
close(INFILE);
close(OUTFILE);
