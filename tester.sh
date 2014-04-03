#! /bin/bash
#
# file to automatically try all tests on a file for the Lang proc project

for f in $(ls tests/test*.prog)
do
	echo -e "$f\t" $(./$1 $f out)
done

