#!/bin/sh

THIS="$0"
if [ ${THIS:0:1} != "/" ] ; then
	if [ -e "$PWD/$THIS" ] ; then
		THIS="$PWD/$THIS"
	fi
fi

THISDIR=$(pwd | sed "s/^.*presets\/\?//" )
OUT=Makefile.am
HERE=$(pwd)

echo -n "SUBDIRS =" > $OUT
echo -e "Makefile\nMakefile.in\n" > .cvsignore


find -type d -mindepth 1 -maxdepth 1 | sed 's/^\.\///' | sed 's/\/$//' | grep -v CVS | \
  while read line; do

    echo -n " $line" >> $OUT
	
    cd "$line"
    "$THIS"
    cd "$HERE"

  done

echo >> $OUT


if [ -n "$THISDIR" ] ; then

	echo -n "EXTRA_DIST =" >> $OUT


	ls *.krp 2> /dev/null | while read line; do
		echo -n " \"$line\"" >> $OUT
	done

	echo >> $OUT

	echo -e "\ninstall-data-local:" >> $OUT
	echo "	\$(mkinstalldirs) \"\$(DESTDIR)\$(kde_datadir)/kradio/presets/$THISDIR/\"" >> $OUT
	
	find -mindepth 1 -maxdepth 1 -name "*.krp" | sed 's/^\.\///' | \
	  while read line; do
		
	    echo "	\$(INSTALL_DATA) \"\$(srcdir)/$line\" \"\$(DESTDIR)\$(kde_datadir)/kradio/presets/$THISDIR/$line\"" >> $OUT

	  done


	echo -e "\n\nuninstall-local:" >> $OUT

	find -mindepth 1 -maxdepth 1 -name "*.krp" | sed 's/^\.\///' | \
	  while read line; do

		echo "	-rm -f \"\$(DESTDIR)\$(kde_datadir)/kradio/presets/$THISDIR/$line\"" >> $OUT	

	  done

fi
