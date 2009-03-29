#!/bin/sh

$EXTRACTRC    *.rc *.ui *.kcfg >> rc.cpp
#$PREPARETIPS   > tips.cpp
$XGETTEXT     *.cpp *.h -o $podir/kradio4_plugin_timeshifter.pot


rm -f rc.cpp tips.cpp

