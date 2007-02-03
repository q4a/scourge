#!/bin/bash
srcdir=.
topdir=$srcdir
domain=scourge
pushd po
make update-po
popd
for lang in `cat $srcdir/po/LINGUAS`
do
  mkdir -p $srcdir/../scourge_data/translations/$lang/LC_MESSAGES
  cp -f $topdir/po/$lang.gmo $srcdir/../scourge_data/translations/$lang/LC_MESSAGES/$domain.mo
done

