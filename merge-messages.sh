#!/bin/bash

ALL_LANGS=$(find -iname "*.po" | while read line ; do basename $line .po ; done | sort -u)

echo "Merging translations"
#catalogs=`find . -name '*.po'`
catalogs=`find -iname "*.pot" | while read pot ; do dir=$(dirname $pot); for LANG in $ALL_LANGS ; do echo $dir/$LANG.po ; done ; done`

for cat in $catalogs; do    
    echo $cat
    catdir=$(dirname "$cat")
    pots=`find "$catdir" -maxdepth 1 -a -name "*.pot"`
    for pot in $pots ; do
        echo "merging $pot into $cat"
        test -e $cat || cat $pot | sed 's/charset=CHARSET/charset=UTF-8/; s/^#, fuzzy/#/' > $cat
        if test -e $cat ; then 
            msgmerge --previous --update -q $cat $pot || exit -1
        else
            msgmerge --previous --update -q -o $cat.new $cat $pot || exit -1
            mv $cat.new $cat
        fi
    done
done
echo "Done merging translations"
