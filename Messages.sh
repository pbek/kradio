#!/bin/sh

$EXTRACTRC    interfaces/*.ui radiostations/*.ui src/*.rc src/*.ui src/*.kcfg >> rc.cpp
#$PREPARETIPS   > tips.cpp
$XGETTEXT     interfaces/*.cpp interfaces/*.h radiostations/*.cpp radiostations/*.h src/*.cpp *.cpp src/*.h -o $podir/kradio.pot

rm -f rc.cpp tips.cpp

