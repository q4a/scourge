#!/bin/sh

for file in *.pcx; do name=`echo $file | awk -F. '{ print $1 }'`; convert $name.pcx -compress None $name.bmp; done

rm -f *_i.*
rm -f weapon.*
rm -f *.pcx
rm -f w_*

