#!/bin/sh
for file in `ls *.pcx`; do name=`echo $file | awk -F. '{ print $1 }'`; convert $name.pcx -compress None $name.bmp; done

