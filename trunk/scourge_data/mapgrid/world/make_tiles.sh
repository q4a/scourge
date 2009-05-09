#!/bin/sh

convert map.png +gravity -crop 128x128 map_%d.png

for i in {0..9}; do
  mv -f map_$i.png map_0$i.png
done
