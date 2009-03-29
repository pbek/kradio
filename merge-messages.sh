#!/bin/bash

echo "Merging translations"
catalogs=`find . -name '*.po'`
for cat in $catalogs; do
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
