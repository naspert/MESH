#! /bin/bash
# $Id: subdiv.sh,v 1.6 2003/05/08 06:41:49 aspert Exp $


bin_path=/home/sun1/aspert/devel/lib3d/bin

if [[ -z "$1" ]]
then
    if [[ $# -ne 3 && $# -ne 4 && $# -ne 5 ]]
    then
    	echo "Usage: `basename $0` [-bin] orig_model subdiv_model nlev [logfile]"
    	exit -1
    fi
fi

if [[ "$1" == "-bin" ]]
then
	sub_options="-bin"
	or_model=$2
	sub_model_root=$3
	nlev=$4
	if [[ $# -eq 5 ]]
        then
          logfile=$5
        else
          logfile="/dev/null"
        fi
else
	sub_options=""
	or_model=$1
	sub_model_root=$2
	nlev=$3
	if [[ $# -eq 4 ]]
	then
	  logfile=$4
	else
	  logfile="/dev/null"
	fi
fi
if [[ ! -e $logfile ]] 
then
  touch $logfile
fi

if [[ -e ${bin_path}/subdiv && -x ${bin_path}/subdiv ]]
then
    sub_exec=${bin_path}/subdiv
else
    if [[ -e Makefile ]]
    then
	echo "Try to build subdiv..."
	make subdiv
	if [[ $? -ne 0 ]] 
	then
	   echo "Attempt to build subdiv failed"
	   exit -1
	fi
	sub_exec=${bin_path}/subdiv
    else
	echo "Unable to find/build subdiv"
	exit -1
    fi
fi

echo "Starting subdivision..."
for meth in "-but" "-sph_or" "-sph_alt" "-loop"
do
  prev_model=$or_model
  tmp=`echo $meth | sed 's/^\-//'`
  suffix=${tmp:0:1}
  case "$meth" in
        "-but" )
        echo -n "Method : Butterfly ... "
	;;

        "-sph_or" )
        echo -n "Method : Spherical (or.)... "
	suffix="spo"
	;;

	"-sph_alt" )
	suffix="spa"
        echo -n "Method : Spherical (alt.)... "
        ;;

        "-loop" )
        echo -n "Method : Loop ... "
	;;
  esac
  for ((i=1; i<=nlev; i++))
  do
     if [[ $sub_options == "-bin" ]]
     then
	     outfile=${sub_model_root}_${suffix}${i}.rawb
     else 
	     outfile=${sub_model_root}_${suffix}${i}.raw
     fi
     $sub_exec $sub_options $meth $prev_model $outfile >> $logfile 2>&1
     prev_model=$outfile
  done
  echo "done"
done
