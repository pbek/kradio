#!/bin/sh

$EXTRACTRC    *.rc *.ui *.kcfg >> rc.cpp
#$PREPARETIPS   > tips.cpp
$XGETTEXT     *.cpp *.h -o $podir/kradio5_plugin_internetradio.pot


rm -f rc.cpp tips.cpp

