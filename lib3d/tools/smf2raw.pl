#! /usr/bin/perl -w
# $Id: smf2raw.pl,v 1.1 2001/03/12 14:50:32 aspert Exp $

if ($#ARGV != 0 && $#ARGV != 1) {
  die("Usage: smf2raw infile[.smf] [outfile.raw]\n");
}

if ($#ARGV == 0) {
  $infile = $ARGV[0];
  if ($infile =~ /.smf/) {
    $tmp = $infile;
    $tmp =~ s/.smf//;
    $outfile = $tmp.".raw";
  }
} else {
  $infile = $ARGV[0];
  if ($infile !~ /.smf/) {
    $infile .= ".smf";
  }
  $outfile = $ARGV[1];
  if ($outfile !~ /.raw/) {
    $outfile .= ".raw";
  }
}

if (-s $infile && -r $infile) {
  open(INFILE, $infile) || die("Unable to open input file $infile\n");
} else {
  die("Input file $infile does not exist or is not readable\n");
}

open(OUTFILE,">$outfile")||die("Unable to open output file $outfile\n");

$vcount = 0;
$fcount = 0;
$vertnum = -1;
$facenum = -1;
@face_arr = ();
@vert_arr = ();
while(<INFILE>) {
  chomp;
  if (/\#/) {
    if (/\$vertices/){
      @tmp_ar = split(/\ /,$_);
      $vertnum = $tmp_ar[$#tmp_ar];
      print "$vertnum vertices in model\n";
    } elsif (/\$faces/){
      @tmp_ar = split(/\ /,$_);
      $facenum = $tmp_ar[$#tmp_ar];
      print "$facenum faces in model\n";
    }
  } elsif (/^v/) {#beginning of vertices
    s/^v\ //;
    
    push(@vert_arr, $_);
    $vcount++;
  } elsif (/^f/) {
    s/^f\ //;
    @tmp = split(/\ /,$_);
    if ($#tmp != 2) {
      die("Error reading face list\n");
    }
    $tmp[0] --;#Our vertex index starts at 0
    $tmp[1] --;
    $tmp[2] --;
    $p = "$tmp[0] $tmp[1] $tmp[2]";
    push(@face_arr, $p);
    $fcount++;
  }
}
#if ($vcount != $vertnum || $fcount != $facenum) {
#  printf "vcount = $vcount vertnum = $vertnum\n";
#  printf "fcount = $fcount facenum = $facenum\n";
#  die("Something went wrong... Header does not correspond to file\n");
#}

print OUTFILE "$vcount $fcount\n";
$i = 0;
while($i < $vcount) {
  print OUTFILE "$vert_arr[$i]\n";
  $i++;
}
$i = 0;
while($i < $fcount) {
  print OUTFILE "$face_arr[$i]\n";
  $i++;
}
close(INFILE);
close(OUTFILE);
