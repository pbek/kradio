#!/bin/sh

$EXTRACTRC    *.rc *.ui *.kcfg >> rc.cpp
#$PREPARETIPS   > tips.cpp
$XGETTEXT     *.cpp *.h -o $podir/kradio5_plugin_gui_quickbar.pot


rm -f rc.cpp tips.cpp

