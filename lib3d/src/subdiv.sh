#! /bin/sh
# $Id: subdiv.sh,v 1.1 2002/03/25 09:55:43 aspert Exp $


bin_path=/home/sun1/aspert/devel/lib3d/bin

if [[ -z "$1" ]]
then
    if [[ $# -ne 3 && $# -ne 4 ]]
    then
    	echo "Usage: `basename $0` orig_model subdiv_model nlev [logfile]"
    	exit -1
    fi
fi

or_model=$1
sub_model_root=$2
nlev=$3
if [[ $# -eq 4 ]]
then
  logfile=$4
else
  logfile="/dev/null"
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
for meth in "-but" "-sph" "-loop"
do
  prev_model=$or_model
  tmp=`echo $meth | sed 's/^\-//'`
  echo -n "Method : $tmp ..."
  for ((i=1; i<=nlev; i++))
  do
     case "$meth" in
	"-but" )
	outfile=${sub_model_root}_b${i}.raw
	;;
   	
	"-sph" )
	outfile=${sub_model_root}_s${i}.raw 
	;;

	"-loop" )
	outfile=${sub_model_root}_l${i}.raw
        ;;
     esac
     $sub_exec $meth $prev_model $outfile > $logfile 2>&1
     prev_model=$outfile
  done
  echo "done"
done
