#!/usr/bin/perl -w
# $Id: raw2wrl.pl,v 1.4 2002/02/01 12:04:10 aspert Exp $

use VRML::VRML2::Standard;

# --------------------------- #
#       WRL file writer       #
# --------------------------- #
sub write_wrl_frame {
    my @pointHolder = @{ $_[0] };    # Array containing vertices list
    my @faceHolder  = @{ $_[1] };    # Array containing face list
    my @normHolder  = @{ $_[2] };
    my $fname       = $_[3];         # filename

    $vrml = new VRML::VRML2::Standard;

    if ( $fname !~ /\.wrl/ ) {
        $fname .= ".wrl";
    }

    $vrml->Shape( sub {
            $vrml->IndexedFaceSet( sub {
                    $vrml->Coordinate(@pointHolder);
                },
                [@faceHolder],
                sub {
                    $vrml->Normal(@normHolder);
                }
            );
        },
        sub {
            $vrml->Appearance( sub {
                    $vrml->Material(
                        'diffuseColor'     => '0.7 0.7 0.7',
                        'ambientIntensity' => '0.7'
                    );
                }
            );
        }
    );
    $vrml->save($fname);
}

# ---------------------------- #
#       Raw file reader        #
# ---------------------------- #
sub read_raw_file {
    my $fname = $_[0];    #input file
    my $numvert;
    my $numface;
    my $i         = 0;
    my $vertArray = \@{ $_[1] };
    my $faceArray = \@{ $_[2] };
    my $normArray = \@{ $_[3] };

    if ( $fname !~ /\.raw/ ) {
        $fname .= ".raw";
    }

    open( INFILE, "$fname" ) || die ("Unable to open $fname\n");

    $header = <INFILE>;
    @tmp    = split ( /\ /, $header );

    if ( $#tmp == 1 ) {
        $numvert = $tmp[0];
        $numface = $tmp[1];
        $numnorm = 0;
        print "$numvert $numface\n";
    }
    elsif ( $#tmp == 2 || $#tmp == 3 ) {
        $numvert = $tmp[0];
        $numface = $tmp[1];
        $numnorm = $tmp[2];
        print "$numvert $numface $numnorm\n";
    }

    print "Reading vertices...\n";
    for ( $i = 0 ; $i < $numvert ; $i++ ) {
        $_ = <INFILE>;
        chomp;
        s/^\s*//;
        s/\s*$//;
        s/\ /,\ /g;
        push ( @$vertArray, $_ );
    }

    print "Reading faces...\n";
    for ( $i = 0 ; $i < $numface ; $i++ ) {
        $_ = <INFILE>;
        s/^\s*//;
        s/\s*$//;
        s/\ /,\ /g;
        push ( @$faceArray, $_ );
    }

    if ( $numnorm > 0 ) {
        print "Reading normals...\n";
        for ( $i = 0 ; $i < $numnorm ; $i++ ) {
            $_ = <INFILE>;
            s/^\s*//;
            s/\s*$//;
            s/\ /,\ /g;
            push ( @$vertArray, $_ );
        }
    }
    close(INFILE);
}

# --------------------------------- #
#               MAIN                #
# --------------------------------- #

my @facearr = ();
my @vertarr = ();
my @normarr = ();
my $j       = 0;

if ( $#ARGV != 1 ) {
    die ("raw2wrl infile[.raw] outfile[.wrl]\n");
}

$fname   = $ARGV[0];
$outfile = $ARGV[1];

if ( $fname !~ /\.raw/ ) {
    $fname .= ".raw";
}

read_raw_file( $fname, \@vertarr, \@facearr, \@normarr );

write_wrl_frame( \@vertarr, \@facearr, \@normarr, $outfile )
