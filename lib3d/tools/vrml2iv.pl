#!/usr/bin/perl -w
# $Id$
#
# ---------------------------------------------------------------------
# This script is intended to convert basic vrml2.0 files
# into SGI's OpenInventor format. 
# WARNING: Only the "point" and "coordIndex" fields will be processed.
# Only the coordinates of the vertices and the connections will be
# written in the output file.
#
# Written by N. Aspert - LTS - EPFL (Nicolas.Aspert@epfl.ch)
# Created on February 8th 2000 - Last modified on February 9th 2000
# --------------------------------------------------------------------



if ($#ARGV < 0) {
  die("Usage: vrml2iv.pl infile[.wrl] [outfile[.iv]]\n");
}

$infile = $ARGV[0];

if ($#ARGV == 1) {
  $outfile = $ARGV[1];
}
else {
  $outfile = $infile;
  $outfile =~ s/(\w*)\.\w*/$1/;
}

if ($infile !~ /\./) {  #add extension
  $infile .= ".wrl";
}

if ($outfile !~ /\./) {
  $outfile .= ".iv";
}

print("Opening input file: $infile\n");
open(INFILE, $infile) || die("Cannot open $infile\n");

#outfile existence check. If it is empty then we use it
if (-s $outfile) {
  close(INFILE);
  die("$outfile already exists... exiting\n");
}

print("Opening output file: $outfile\n");
open(OUTFILE,">$outfile") || die("Error creating $outfile : $!\n");


#print Inventor header
print OUTFILE "#Inventor V2.1 ascii\n\n";
print OUTFILE "Separator {\n";
print OUTFILE "\tShapeHints {\n";
print OUTFILE "\t\tshapeType UNKNOWN_SHAPE_TYPE\n";
print OUTFILE "\t\tcreaseAngle 0.0\n";
print OUTFILE "\t\tvertexOrdering COUNTERCLOCKWISE\n";
print OUTFILE "\t}\n";

$VRML_HEADER = "#VRML V2.0 utf8";
#Read first line of VRML file and check header
$_ = <INFILE>; 
chomp; # get rid of trailing \n

if (!/$VRML_HEADER/) {
  close(INFILE);
  close(OUTFILE);
  die("$infile does not have a valid VRML header:\n$_\n");
}

#Some useful counters & flag keepers...
$vert_beg = 0;
$face_beg = 0;
$vert_num = 0;
$csv = 0;
$face_num = 0;
$vrml_complete = 0;
#Read VRML file

while(<INFILE>) {
#----------------------- Vertices processing -----------------------------
  if (($vert_beg == 1) and !/\]/){ #Vertex read (not first, nor last one)
    s/^\s*//;#Remove leading spaces/tabs
    chomp;
    s/\s*$//;#Remove trailing spaces
    s/,//g;#remove all comas 'cause IV seems to dislike inner commas
    $_ .= ",";#add a comma at the end
    print OUTFILE "\t\t\t$_\n";
    $vert_num ++;
  }

  elsif (/point/ and /\[/ and ($vert_beg != 1)) {
    print "Beginning of vertices list found\n";
    $vert_beg = 1;
  
    #Separate processing of the first vertex that can lie on the same line
    print OUTFILE "\tCoordinate3 {\n";
    print OUTFILE "\t\tpoint [\n";
    #$tmp = $_;
    s/point//;
    s/\[//;#remove "point [" from the string
    s/^\s*//;#remove leading tabs/spaces
    chomp;
    s/\s*$//;

    if ($_ ne "") {
      s/,//g;
      $_ .= ",";#add a comma at the end
      print OUTFILE "\t\t\t$_\n";
      $vert_num ++;
    }
  }
  

  elsif (($vert_beg == 1) and /\]/) {#Last vertex found
    s/\]//;
    s/^\s*//;#Remove leading blanks
    chomp;
    s/\s*$//;#Remove trailing blanks
    
    if ($_ ne "") { #if there is sthg before " ]"
      print OUTFILE "\t\t\t$_,\n";
      $vert_num ++;
    }

    print OUTFILE "\t\t]\n\t}\n";
    print("End of vertices list found\n");
    print("Found $vert_num vertices\n");
    $vert_beg = 0;
    $vrml_complete += 1; 
  }
#-------------------------- End of vertices processing ----------------------

#-------------------------- Face processing --------------------------------
 
  elsif(/coordIndex/ and /\[/) {
    print "Beginning of face list found\n";
    $face_beg = 1;
    s/coordIndex//;
    s/\[//;
    
    #Remove leading and trailing blanks
    s/^\s*//;
    chomp;
    s/\s*$//;

    print OUTFILE "\tIndexedFaceSet {\n";
    print OUTFILE "\t\tcoordIndex [\n";
    
    
    if ($_ ne "") { #If there is something left...
      if (/,/) {
	$csv = 1;#already have csv
      }
      else {
	$csv = 0;#File has no CSV
	s/\ /,\ /g;#add comma...
	if (!/,$/) {#if there is no comma at the end
	  $_ .= ",";
	}
      }
      @tmp = split(/-1,/,$_);
      $face_num += $#tmp +1;
      for ($i = 0;$i <= $#tmp; $i++){
	$tmp[$i] =~ s/^\s*//;
	$tmp[$i] =~ s/\s*$//;
	print OUTFILE "\t\t\t$tmp[$i] -1,\n";
      }
    }
    else {
      $_ = <INFILE>; #read next line to check whether we have csv or not
      chomp;
      s/^\s*//;
      s/\s*$//;
      if (/,/) {
	$csv = 1;#already have csv
      }
      else {
	$csv = 0;#File has no CSV
	s/\ /,\ /g;#add comma...
	if (!/,$/) {#if there is no comma at the end
	  $_ .= ",";
	}
	@tmp = split(/-1,/,$_);
	$face_num += $#tmp +1;
	for ($i = 0;$i <= $#tmp; $i++){
	  $tmp[$i] =~ s/^\s*//;
	  $tmp[$i] =~ s/\s*$//;
	  print OUTFILE "\t\t\t$tmp[$i] -1,\n";
	}
      }
    }
    
  }

 elsif($face_beg==1 and !/\]/){ #If we have not reached the end
    chomp;
    s/^\s*//;
    s/\s*$//;
    
    if ($csv == 0) { 
      s/\ /,\ /g;
      if(!/,$/) {
	$_ .= ",";
      }
    }
    if(!/,$/) {
	$_ .= ",";
      }
    @tmp = split(/-1,/,$_);
    $face_num += $#tmp +1;
    for ($i = 0;$i <= $#tmp; $i++){
      $tmp[$i] =~ s/^\s*//;
      $tmp[$i] =~ s/\s*$//;
      print OUTFILE "\t\t\t$tmp[$i] -1,\n";
    }
  }

 elsif ($face_beg==1 and /\]/) { #process last faces
    chomp;
    s/\]//;
    s/^\s*//;
    s/\s*$//;
    
    if ($_ ne ""){
      if($csv == 0) {
	s/\ /,\ /g;
      }
      if(!/,$/) {
	$_ .= ",";
      }
      @tmp = split(/-1,/,$_);
      $face_num += $#tmp +1;
      for ($i = 0;$i <= $#tmp; $i++){
	$tmp[$i] =~ s/^\s*//;
	$tmp[$i] =~ s/\s*$//;
	print OUTFILE "\t\t\t$tmp[$i] -1,\n";
      }
    }
    print OUTFILE "\t\t]\n\t}\n";#close field
    $face_beg = 0;
    $vrml_complete += 2;
    print "End of face list found\n";
    print "$face_num faces found in model\n";
  }
#---------------- End of face processing ------------------

#We just ignore all the other VRML tags...

}#end while
#All file has been read... check if some tags are not closed...
if ($vrml_complete != 3) {#Error found...
  print "Error code $vrml_complete\n";
  if($vrml_complete == 2) {
    print "Face list not closed in input file: $infile ... \n";
  }
  elsif($vrml_complete == 1) {
    print "Vertex list not closed in input file: $infile ... \n";
  }
  elsif($vrml_complete == 0) {
    print "The file does not contain \"point\" nor \"indexCoord\" fields !\n";
  }
  else {
    print "An error in the VRML file has been found...\n";
    print "Some tags may be missing, or there are more than ";
    print "one tag of the same kind\n";    
  }
  #Common part
  print "Closing files...\n";
  close(INFILE);
  close(OUTFILE);
  print "Removing output file ...\n";
  `rm $outfile`;
  die("Exiting....\n");
}

print OUTFILE "}\n";
print("Closing files\n");
close(INFILE);
close(OUTFILE);
