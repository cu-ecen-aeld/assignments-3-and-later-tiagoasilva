#!/bin/sh
# Assignment 1 - step 9
# Author: Tiago Silva

FILESDIR=.
SEARCHSTR=""

if [ $# -lt 2 ]
then
	echo "Missing filesdir or searchstr parameters. Call example: finder.sh /tmp/aesd/assignment1 linux"
	exit 1
else
	FILESDIR=$1
	SEARCHSTR=$2
fi

if [ -d "$FILESDIR" ]
then
	NUMFILES=$(find $FILESDIR -type f | wc -l)
	MATCHINGLINES=$(grep -r $SEARCHSTR $FILESDIR | wc -l)
	echo "$(printf "The number of files are %d and the number of matching lines are %d" "$NUMFILES" "$MATCHINGLINES")" 
else
	echo "The path "$FILESDIR" doesn't exist!"
	exit 1
fi

