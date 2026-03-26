#!/bin/sh
# Writer script for assignament 1
# Author: Tiago Silva

WRITEFILE=""
WRITESTR=""

if [ $# -lt 2 ]
then
	echo "You need to pass the file and string. Example: writer.sh /tmp/aesd/assignment1/sample.txt ios"
	exit 1
else
	WRITEFILE=$1
	WRITESTR=$2
fi

DIRNAME=$(dirname "$WRITEFILE")
if [ ! -d "$DIRNAME" ]
then
	mkdir -p "$DIRNAME"
	if [ $? -ne 0 ]
	then
		echo "Error creating the path"
		exit 1
	fi
fi

echo "$WRITESTR" > "$WRITEFILE"
if [ $? -ne 0 ]
then
	echo "Error generating file"
	exit 1
fi

