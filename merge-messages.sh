#!/bin/bash

MIN_LANGS="cs de es pl pt ru uk"

echo "Merging translations"
#catalogs=`find . -name '*.po'`
catalogs=`find -iname "de.po" -o -name "*.po" | while read line ; do echo $line ; for LANG in $MIN_LANGS ; do echo $line | sed "s:de\.po:$LANG.po:" ; done ; done | sort | uniq`

for cat in $catalogs; do
    test -e $cat || touch $cat
    echo $cat
    catdir=$(dirname "$cat")
    pots=`find "$catdir" -maxdepth 1 -a -name "*.pot"`
    for pot in $pots ; do
        echo "merging $pot into $cat"
        msgmerge -o $cat.new $cat $pot
        mv $cat.new $cat
    done
done
echo "Done merging translations"
