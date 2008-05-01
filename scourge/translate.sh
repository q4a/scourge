#!/bin/bash
srcdir=.
topdir=$srcdir
domain=scourge

echo 'Creating po/POTFILES.in ...'

 ( find src -name '*.cpp' -print && find src -name '*.h' -print && find ../scourge_data/config -name '*.cfg' -print && find ../scourge_data/maps -name '*.cfg' -print && find ../scourge_data/script -name '*.nut' -print ) > po/POTFILES.in

pushd po
make update-po
popd
for lang in `cat $srcdir/po/LINGUAS`
do
  mkdir -p $srcdir/../scourge_data/translations/$lang/LC_MESSAGES
  cp -f $topdir/po/$lang.gmo $srcdir/../scourge_data/translations/$lang/LC_MESSAGES/$domain.mo
done

