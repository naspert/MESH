#!/usr/bin/perl -w
# $Id: raw2wrl.pl,v 1.1 2001/03/12 14:50:32 aspert Exp $

use VRML::VRML2::Standard;

# This sub writes a .wrl file 
sub write_wrl_frame {
  my @pointHolder = @{$_[0]}; # Array containing vertices list
  my @faceHolder = @{$_[1]}; # Array containing face list
  my $fname = $_[2]; # filename

  $vrml = new VRML::VRML2::Standard;
  if ($fname !~ /\.wrl/) {
    $fname .= ".wrl";
  }
  $vrml->Shape(
	       sub{    
		 $vrml->IndexedFaceSet(sub {
					 $vrml->Coordinate(@pointHolder);
				       }, [@faceHolder])
	       } ,
	       sub{    
		 $vrml->Appearance(sub{
				     $vrml->Material('diffuseColor'=> 
						     '0.7 0.7 0.7',
						     'ambientIntensity' => 
						     '0.7')
				   })
	       }
	      );
  $vrml->save($fname);
}
#---------------------------------------------------------------------------


sub read_raw_file {
  my $fname = $_[0]; #input file
  my $numvert;
  my $numface;
  my $i = 0;
  my @facearr = @{$_[1]};
  my @vertarr = @{$_[2]};
  my $j = 0;

  if ($fname !~ /\.raw/) {
    $fname .= ".raw";
  }

  open(INFILE, "$fname") || die ("Unable to open $fname\n");
  
  $header = <INFILE>;
  @tmp = split(/\ /,$header);
  if ($#tmp == 1) {
    $numvert = $tmp[0];
    $numface = $tmp[1];
    print "$numvert $numface\n";
  }
  while ($i < $numvert) {
    $_ = <INFILE>;
    chomp;
#    print "$_";
    s/^\s*//;
    s/\s*$//;
    @tmp = split(/\ /);
    $j = 0;
    while ($j <= $#tmp) {
      push(@vertarr, $tmp[$j]);
#      print "$tmp[$j]\n";
      $j++;
    }
    $i++
  }
  $i = 0;
  while ($i < $numface) {
    <INFILE>;
    s/^\s*//;
    s/\s*$//;
    @tmp = split(/\ /);
    $j = 0;
    while ($j <= $#tmp) {
      push(@facearr, $tmp[$j]);
      $j++;
    }
    $i++
  }
  close(INFILE);
}


#------------ MAIN ----------------
my @facearr = ();
my @vertarr = ();

if ($#ARGV != 1) {
  die("raw2wrl infile[.raw] outfile[.wrl]\n");
}

$fname  = $ARGV[0];
$outfile = $ARGV[1];
#read_raw_file($infile, \@facearr, \@vertarr);
if ($fname !~ /\.raw/) {
  $fname .= ".raw";
}

open(INFILE, "$fname") || die ("Unable to open $fname\n");

$header = <INFILE>;
@tmp = split(/\ /,$header);
if ($#tmp == 1) {
  $numvert = $tmp[0];
  $numface = $tmp[1];
  print "$numvert $numface\n";
}
$i = 0;
$j = 0;
while ($i < $numvert) {#vertex loop
  $_ = <INFILE>;
  chomp;
  s/^\s*//;
  s/\s*$//;
  s/\ /,\ /g;
  push(@vertarr, $_);
  $i++
}
$i = 0;
while ($i < $numface) {#face loop
  $_ = <INFILE>;
  chomp;
  s/^\s*//;
  s/\s*$//;
  s/\ /,\ /g;
  push(@facearr, $_);
  $i++
}

close(INFILE);

write_wrl_frame(\@vertarr, \@facearr, $outfile)
