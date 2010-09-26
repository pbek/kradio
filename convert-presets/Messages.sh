#!/bin/sh

$EXTRACTRC    *.rc *.ui *.kcfg >> rc.cpp
#$PREPARETIPS   > tips.cpp
$XGETTEXT     *.cpp -o $podir/kradio4_convert_presets.pot


rm -f rc.cpp tips.cpp

