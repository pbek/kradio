#!/bin/sh

THIS="$0"
if [ ${THIS:0:1} != "/" ] ; then
	if [ -e "$PWD/$THIS" ] ; then
		THIS="$PWD/$THIS"
	fi
fi

THISDIR=$(pwd | sed "s/^.*presets\/\?//" )
OUT=CMakeLists.txt
HERE=$(pwd)

echo -n "" > $OUT

find -mindepth 1 -maxdepth 1 -type d | sed 's/^\.\///' | sed 's/\/$//' | egrep -v "\.svn" | \
  while read line; do

    echo "ADD_SUBDIRECTORY($line)" >> $OUT
	
    cd "$line"
    "$THIS"
    cd "$HERE"

  done

echo >> $OUT

N_KRP=$(ls *.krp 2>/dev/null | wc -l)
if [ -n "$THISDIR" ] && [ $N_KRP -gt 0 ] ; then

    echo "file(GLOB presets  \${CMAKE_CURRENT_SOURCE_DIR}/*.krp)"                               >> $OUT

    echo "install(FILES \${presets} DESTINATION \${DATA_INSTALL_DIR}/kradio4/presets/$THISDIR )" >> $OUT

fi
