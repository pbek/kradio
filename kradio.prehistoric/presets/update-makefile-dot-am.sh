#!/bin/sh

OUT=Makefile.am
THISDIR=$(pwd | sed "s/^.*presets\/\?//" )
THIS=$(basename $0)
HERE=$(pwd)

echo -n "SUBDIRS =" > $OUT

find -type d -mindepth 1 -maxdepth 1 | sed 's/^\.\///' | sed 's/\/$//' | grep -v CVS | \
  while read line; do

    echo -n " $line" >> $OUT
    cp "$0" "$line"
	
    cd "$line"
    "./$THIS"
    cd "$HERE"

  done


echo $THISDIR
if [ -n "$THISDIR" ] ; then

	echo "EXTRA_DIST =" *.krp > $OUT

	echo -e "\ninstall-data-local:" >> $OUT
	echo "	\$(mkinstalldirs) \$(kde_datadir)/kradio/presets/$THISDIR/" >> $OUT
	
	find -mindepth 1 -maxdepth 1 -name "*.krp" | sed 's/^\.\///' | \
	  while read line; do
		
	    echo "	\$(INSTALL_DATA) \$(srcdir)/$line \$(kde_datadir)/kradio/presets/$THISDIR/$line" >> $OUT

	  done


	echo -e "\n\nuninstall-local:" >> $OUT

	find -mindepth 1 -maxdepth 1 -name "*.krp" | sed 's/^\.\///' | \
	  while read line; do

		echo "	-rm -f \$(kde_datadir)/kradio/presets/$THISDIR/$line" >> $OUT	

	  done

fi
