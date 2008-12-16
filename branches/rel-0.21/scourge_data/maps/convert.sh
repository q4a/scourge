#!/bin/bash

for f in *.txt
do
	NAME=`echo $f| awk -F. '{print $1}'`
	echo $NAME
	./convert $NAME.txt 2>$NAME.cfg
done

