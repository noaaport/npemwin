#!/bin/sh

range_to_spacelist () {
    #
    # The input is a dash separated string, then converted to a space
    # separated string that is input to seq. (Single digit integers are
    # padded with a zero in the output.)
    #
    range=$1
    set `echo $range | tr "-" " "`
    first=$1
    last=$2

    nlist=`seq -s " " $first $last`

    nnlist=
    for n in $nlist
    do
	nn=`printf "%02d" $n`
	nnlist="$nnlist $nn"
    done

    echo $nnlist
}

rangelist_to_spacelist () {
    #
    # The input is a comma separated string of "ranges". Each range
    # is converted to a space separated sequence with "seq" (via the
    # function range_to_spacelist ().
    #
    # Example: convert a list of the form
    #
    # 1-1,3-5,9-9
    #
    # to
    #
    # 01 03 04 05 09
    #
    rangelist=$1

    # initialize the final list
    nnlist=
    
    # get each range in the comma-separated list
    for range in `echo $rangelist | tr "," " "`
    do
	# convert each range with "seq"
	spacelist=`range_to_spacelist $range`

	# add it to the final list
	nnlist="$nnlist $spacelist"
    done

    echo $nnlist
}

#
# Test
#
[ $# -eq 0 ] && { echo "list?"; exit 1; }
rangelist=$1

nnlist=`rangelist_to_spacelist $rangelist`
echo $nnlist
