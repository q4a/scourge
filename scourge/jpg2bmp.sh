#!/bin/sh
for file in `ls *.jpg`; do name=`echo $file | awk -F. '{ print $1 }'`; convert $file -compress None -resize 256 $name.bmp; done

