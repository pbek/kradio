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

#include <qstringlist.h>

#include <kradio/interfaces/radio_interfaces.h>
#include <kradio/interfaces/stationselection_interfaces.h>
#include <kradio/libkradio/stationlist.h>

#include "stationselector-ui.h"
#include "radiostation-listview.h"


class RadioStationListView;

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

    bool noticePowerChanged(bool /*on*/)                          { return false; }
    bool noticeStationChanged (const RadioStation &, int /*idx*/) { return false; }
    bool noticeStationsChanged(const StationList &sl);
    bool noticePresetFileChanged(const QString &/*f*/)            { return false; }

    bool noticeCurrentSoundStreamIDChanged(SoundStreamID)         { return false; }

    void   saveState    (KConfig *) const;
    void   restoreState (KConfig *);

protected slots:

    void slotButtonToLeft();
    void slotButtonToRight();

    void slotOK();
    void slotCancel();

protected:

    void moveItem (RadioStationListView *fromListView,    QStringList          &fromIDList,
                   QListViewItem        *item,            int                   fromIdx,
                   RadioStationListView *toListView,      QStringList          &toIDList);

    void updateListViews();

    // station ids
    QStringList   m_stationIDsAvailable,
                  m_stationIDsSelected,
                  m_stationIDsNotDisplayed,
                  m_stationIDsAll;
};

#endif
