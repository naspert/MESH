#!/usr/bin/perl -w
# $Id$
use VRML::VRML2::Standard;

# This sub writes a .wrl file 
sub write_wrl_frame {
  my @pointHolder = @{$_[0]}; # Array containing vertices list
  my @faceHolder = @{$_[1]}; # Array containing face list
  my $frnum = $_[2]; # Current frame number
  my $fname = $_[3]; # Base filename
  my $objnum = $_[4]; # Object number (only useful if more than one, but...;-)
  if ($frnum == -1) {
    $outname = $fname."_o"."$objnum"."_base.wrl";
  } else {
    $outname = $fname."_o"."$objnum"."_$frnum.wrl";
  }
  $vrml = new VRML::VRML2::Standard;

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
  $vrml->save($outname);
}
#--------------------------------------------------------------------------------

# This sub writes a .raw file 
sub write_raw_file {
  my @pointHolder = @{$_[0]}; # Array containing vertices list
  my @faceHolder = @{$_[1]}; # Array containing face list
  my $frnum = $_[2]; # Current frame number
  my $fname = $_[3]; # Base filename
  my $vxnum = $_[4]; # Number of vertices/model
  my $fcnum = $_[5]; # Number of faces/model
  my $objnum = $_[6]; # object number
  my $i = 0;

  if ($frnum == -1) {
    $out = $fname."_o"."$objnum"."_base.raw";
  } else {
    $out = $fname."_o"."$objnum"."_$frnum.raw";
  }
  print "$out\n";
  open(OUTFILE,">$out")||die("Unable to open output file $out\n");
  print OUTFILE "$vxnum $fcnum\n";
  
  while ($i < $vxnum) {
    if ($frnum == -1) {
      $tmp = $pointHolder[$i];
    } else {
      $tmp = $pointHolder[$frnum*$vxnum+$i];
    }
    $tmp =~ s/,//g;
    print OUTFILE "$tmp\n";
    $i++;
  }
  $i = 0;
  while ($i < $fcnum) {
    $tmp=$faceHolder[$i];
    $tmp =~ s/,//g;
    print OUTFILE "$tmp\n";
    $i++
  }
  close(OUTFILE);
}
# ------------------------------------------------------------


# ----------------------- 
# Main prog begins here
# -----------------------

if ($#ARGV != 2 && $#ARGV !=1) {
  die("process.pl [-dump-rawfiles] infile outfile_base[#.wrl]\n");
}

if ($ARGV[0] =~ /\-dump\-rawfiles/) {
  $dump = 1;
  $infile = $ARGV[1];
  $outfile = $ARGV[2];
}else {
  $dump = 0;
  $infile = $ARGV[0];
  $outfile = $ARGV[1];
}


if (-s $infile && -r $infile) {
  open(INFILE, $infile) || die("Unable to open input file $infile\n");
} else {
  die("Input file $infile does not exist or is not readable\n");
}


#Some useful variables...
$vertnum = 0;
$facenum = 0;
$framenum = 0;
$vertfnum = 0;
$frame1cplt = 0;
@pointArr = ();#Storage for vertices coord
@faceArr = ();#Storage for face list
@framevArr = (); #Storage for moving vertices

$point_ff = 0;
$coord_ff = 0;
$key_ff = 0;
$keyv_ff = 0;

# Main loop for parser
while(<INFILE>) {
  if (/point/ && /\[/) {
    print "point field found\n";
    $point_ff++;
    if ($point_ff > 1) {
      #write da wrl frames....
      write_wrl_frame(\@pointArr, \@faceArr, -1, $outfile, $point_ff-1);
      $curfr = 0;
      @frameArr = ();
      while ($curfr < $framenum) {
	@frameArr = ();
	$k = 0;
	while ($k < $vertnum) {
	  push(@frameArr,  $framevArr[$curfr*$vertnum + $k]);
	  $k++;
	}
	#  print "$#frameArr\n";
	write_wrl_frame(\@frameArr, \@faceArr, $curfr, $outfile, $point_ff-1);
	$curfr++,
      }
      if ($dump ==1) {
	print "Saving raw\n";
	#save da base model
	write_raw_file(\@pointArr, \@faceArr, -1, $outfile, $vertnum, 
		       $facenum, $point_ff-1);
	$curfr = 0;
	while ($curfr < $framenum) {
	  write_raw_file(\@framevArr, \@faceArr,  $curfr, $outfile, $vertnum, 
			 $facenum, $point_ff-1);
	  
	  $curfr++;
	}
      }
      #reset fields
      $vertnum = 0;
      $facenum = 0;
      $framenum = 0;
      $vertfnum = 0;
      $frame1cplt = 0;
      @pointArr = ();#Storage for vertices coord
      @faceArr = ();#Storage for face list
      @tmpArr = ();
      @framevArr = (); #Storage for moving vertices
    }
    $_ = <INFILE>;
    while ($_ !~ /\]/) {
      chomp;
      s/^\s*//;
      s/\s*$//;

      s/,$//;#remove final ,
      @tmp = split(/,/,$_);
      $i = 0;
      while ($i <= $#tmp) {
	$tmp[$i] =~ s/^\s*//;
	$tmp[$i] =~ s/\s*$//;
	push(@pointArr,$tmp[$i]);
	$vertnum++;
	$i++;
      }
      $_ = <INFILE>;
    }
    s/\]$//;
    @tmp = split(/,/,$_);
    $i = 0;
    while ($i <= $#tmp) {
      $tmp[$i] =~ s/^\s*//;
      $tmp[$i] =~ s/\s*$//;
      push(@pointArr, $tmp[$i]);
      $vertnum++;
      $i++;
    }
    $frame1cplt++;
  } elsif (/coordIndex/ && /\[/) {
    print "coordIndex field found\n";
    $coord_ff++;
    $_ = <INFILE>;
    while ($_ !~ /\]/) {
      chomp;
      s/^\s*//;
      s/\s*$//;
      s/,$//;
      @tmp = split(/-1/,$_);
      $i = 0;
      while ($i <= $#tmp) {
	$tmp[$i] =~ s/^,//;
	$tmp[$i] =~ s/^\s*//;
	$tmp[$i] =~ s/\s*$//;
	$tmp[$i] =~ s/,$//;
	@tmp2 = split(/\ /,$tmp[$i]);
	$j = 0;
	while ($j <= $#tmp2) {
	  push(@tmpArr,$tmp2[$j]);
	  $j++;
	}
	$facenum++;
	$i++;
      }
      $_ = <INFILE>;
    }
    s/^\s*//;
    s/\s*$//;
    s/\]//;
    s/,$//;
    @tmp = split(/-1/,$_);
    $i = 0;
    while ($i <= $#tmp) {
      $tmp[$i] =~ s/^,//;
      $tmp[$i] =~ s/^\s*//;
      $tmp[$i] =~ s/\s*$//;
      $tmp[$i] =~ s/,$//;
      @tmp2 = split(/\ /, $tmp);
      $j = 0;
      while ($j <= $#tmp2) {
	push(@tmpArr, $tmp2[$j]);
	$j++;
      }
      $facenum++;
      $i++;
    }
    $frame1cplt += 2;
  } elsif (/key/ && !/keyValue/ && /\[/ && ($key_ff==$point_ff-1)) {
    print "key field found\n";
    $key_ff++;
    if (/,/) {
      s/\[//;
      s/key//;
      s/^\s*//;
      s/\s*$//;   
      s/,$//;#remove final ,
      @tmp = split(/,/,$_);
      $i = 0;
      while ($i <= $#tmp) {
	$tmp[$i] =~ s/^\s*//;
	$tmp[$i] =~ s/\s*$//;
	$framenum++;
	$i++;
      }
    }
    $_ = <INFILE>;
    while ($_ !~ /\]/) {
      chomp;
      s/^\s*//;
      s/\s*$//;   
      s/,$//;#remove final ,
      @tmp = split(/,/,$_);
      $i = 0;
      while ($i <= $#tmp) {
	$tmp[$i] =~ s/^\s*//;
	$tmp[$i] =~ s/\s*$//;
	$framenum++;
	$i++;
      }
      $_ = <INFILE>;
    }
    s/^\s*//;
    s/\s*$//;
    s/\]$//;
    s/^\s*//;
    s/\s*$//;
    s/,$//;
    s/^\s*//;
    s/\s*$//;
    @tmp = split(/,/,$_);
    $i = 0;
    while ($i <= $#tmp) {
      $tmp[$i] =~ s/^\s*//;
      $tmp[$i] =~ s/\s*$//;
      $framenum++;
      $i++;
    }
  } elsif (/keyValue/ && /\[/ && ($keyv_ff==$point_ff-1)) {
    print "keyValue field found\n";
    $keyv_ff++;
    if (/,/) {#values contained in the first line
      s/\[//;
      s/keyValue//;
      s/^\s*//;
      s/\s*$//;
      s/,$//;
      @tmp = split(/,/,$_);
      $i = 0;
      while ($i <= $#tmp) {
	$tmp[$i] =~ s/^\s*//;
	$tmp[$i] =~ s/\s*$//;
	push(@framevArr, $tmp[$i]);
	$vertfnum++;
	$i++;
      }
    }
    $_ = <INFILE>;
    while ($_ !~ /\]/) {
      chomp;
      s/^\s*//;
      s/\s*$//;
      s/,$//;#remove final ,
      @tmp = split(/,/,$_);
      $i = 0;
      while ($i <= $#tmp) {
	$tmp[$i] =~ s/^\s*//;
	$tmp[$i] =~ s/\s*$//;
	push(@framevArr, $tmp[$i]);
	$vertfnum++;
	$i++;
      }
      $_ = <INFILE>;
    }
    s/^\s*//;
    s/\s*$//;
    s/\]$//;
    s/^\s*//;
    s/\s*$//;
    s/,$//;
    s/^\s*//;
    s/\s*$//;
    @tmp = split(/,/,$_);
    $i = 0;
    while ($i <= $#tmp) {
      $tmp[$i] =~ s/^\s*//;
      $tmp[$i] =~ s/\s*$//;
      push(@framevArr, $tmp[$i]);
      $vertfnum++;
      $i++;
    }
  }
  
}
close(INFILE);

# build face Arr (assume we have triangles)
$i = 0;
$facenum = 0;
while ($i < $#tmpArr) {
  $tmp = "";
  $tmp = sprintf("%d %d %d", $tmpArr[$i], $tmpArr[$i+1], $tmpArr[$i+2]);
  print "$tmp\n";
  $i += 3;
  push(@faceArr, $tmp);
  $facenum++;
  
}


if ($point_ff == 1) {
  #write da wrl frames....
  write_wrl_frame(\@pointArr, \@faceArr, -1, $outfile, $point_ff);
  $curfr = 0;
  @frameArr = ();
  while ($curfr < $framenum) {
    @frameArr = ();
    $k = 0;
    while ($k < $vertnum) {
      push(@frameArr,  $framevArr[$curfr*$vertnum + $k]);
      $k++;
    }
    #  print "$#frameArr\n";
    write_wrl_frame(\@frameArr, \@faceArr, $curfr, $outfile, $point_ff);
    $curfr++,
  }
  if ($dump == 1) {
    print "Saving raw\n";
    #save da base model
    write_raw_file(\@pointArr, \@faceArr, -1, $outfile, $vertnum, $facenum, 
		   $point_ff);
    $curfr = 0;
    while ($curfr < $framenum) {
      write_raw_file(\@framevArr, \@faceArr,  $curfr, $outfile, $vertnum, 
		     $facenum, $point_ff);
      
      $curfr++;
    }
  }
}
print "$vertnum vertices processed and $facenum faces found\n";
print "$vertfnum vertices found in $framenum frames\n";
#die("Temp stop\n");





