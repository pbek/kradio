/***************************************************************************
                          StationSelector.h  -  description
                             -------------------
    begin                : Son Aug 3 2003
    copyright            : (C) 2003 by Martin Witte
    email                : witte@kawo1.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_STATIONSELECTOR_H
#define KRADIO_STATIONSELECTOR_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "stationselector-ui.h"
#include "radio_interfaces.h"
#include "stationselection_interfaces.h"

#include <vector>
using std::vector;

class KListBox;

class StationSelector : public StationSelectorUI,
                        public IRadioClient,
                        public IStationSelectionClient
{
Q_OBJECT
public :
    StationSelector (QWidget *parent);
    ~StationSelector ();

    bool connectI (Interface *i);
    bool disconnectI (Interface *i);

// IStationSelectionClient

    bool noticeStationSelectionChanged(const QStringList &sl);

// IRadioClient

    bool noticePowerChanged(bool on);
    bool noticeStationChanged (const RadioStation &, int idx);
    bool noticeStationsChanged(const StationList &sl);
    bool noticePresetFileChanged(const QString &/*f*/)           { return false; }

protected slots:

    void slotButtonToLeft();
    void slotButtonToRight();

    void slotOK();
    void slotCancel();

protected:

    void moveItem (KListBox *&lbFrom, vector<QString> &vFrom,
                   unsigned int idxFrom,
                   KListBox *&lbTo, vector<QString> &vTo);
    void insertItem (KListBox *&lb, vector<QString> &v, const QString &id, const QPixmap *p, const QString &txt);

    vector<QString>   stationsAvailable,
                      stationsSelected,
                      stationsNotDisplayed,
                      stationsAll;


};

#endif
