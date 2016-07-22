#!/bin/bash
# Script to extract the current conditional compilation marks
#   which are in used to build the Rand editor.
# The output goes into the specified file, or by default
#   in /tmp/Rand_condition
# By Fabien Perriollat CERN/PS <Fabien.Perriollat@cern.ch>
#   last edit : March 2002

DefaultFile="/tmp/Rand_compilation_conditions"
synopsis="synopsis : $0 [-h] [--help] [output file name]"

if [ $# -gt 0 ]; then
    if test $1 = "-h" -o $1 = "--help"; then
	echo "Script to extract the current set of conditional compilation marks"
	echo $synopsis
	echo "  Default output file : $DefaultFile"
	exit 0;
    else
	if [ $# -gt 1 ]; then
	    echo $0 : Invalid parameters \"$@\"
	    echo $synopsis
	    exit 1;
	fi;
    fi;
fi;

OutupFile=${1:-$DefaultFile}

echo "$0 : Result into : \"${OutupFile}\""

cd e19
  cc -I. -I../include -I../la1 -I../ff3 -E -dM e.h | \
    sed -n "/^#define [^_][^ _[:lower:]]*$/ s/^#define // p" | \
    sort -o ${OutupFile} -
cd ..

