/***************************************************************************
                          StationSelector.h  -  description
                             -------------------
    begin                : Son Aug 3 2003
    copyright            : (C) 2003 by Martin Witte
    email                : emw-kradio@nocabal.de
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

#include <QStringList>
#include <QWidget>

#include "radio_interfaces.h"
#include "stationselection_interfaces.h"
#include "stationlist.h"

class RadioStationListView;
class Ui_StationSelectorUI;
class KConfigGroup;

class KDE_EXPORT StationSelector : public QWidget,
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

    bool noticeRDSStateChanged      (bool  /*enabled*/)           { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)        { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/)        { return false; }

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID)   { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID)   { return false; }

    void   saveState    (KConfigGroup &) const;
    void   restoreState (KConfigGroup &);

    bool   isDirty () const { return m_dirty; }

protected slots:

    void slotButtonToLeft();
    void slotButtonToRight();

    void slotOK();
    void slotCancel();
    void slotSetDirty();

signals:

    void sigDirty();

protected:

    void moveSelectedRows(RadioStationListView *fromListView,
                          RadioStationListView *toListView);

    void updateListViews();

    // station ids
    QStringList   m_stationIDsAvailable,
                  m_stationIDsSelected,
                  m_stationIDsNotDisplayed,
                  m_stationIDsAll;

    bool          m_dirty;

    Ui_StationSelectorUI *m_ui;
};

#endif
