/***************************************************************************
                          v4lradio-configuration.h  -  description
                             -------------------
    begin                : Fre Jun 20 2003
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

#ifndef KRADIO_TIMESHIFTER_CONFIGURATION_H
#define KRADIO_TIMESHIFTER_CONFIGURATION_H

#include <QWidget>
#include <QComboBox>

#include "soundstreamclient_interfaces.h"
#include "gui_list_helper.h"

#include "ui_timeshifter-configuration-ui.h"

class TimeShifter;

class TimeShifterConfiguration : public QWidget,
                                 public Ui_TimeShifterConfigurationUI,
                                 public ISoundStreamClient
{
Q_OBJECT
public :
    TimeShifterConfiguration (QWidget *parent, TimeShifter *shifter);
    ~TimeShifterConfiguration ();

    bool connectI (Interface *i);
    bool disconnectI (Interface *i);

    void noticeConnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid);
    void noticeDisconnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid);

// ISoundStreamClient

RECEIVERS:
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid);
    bool noticePlaybackChannelsChanged(const QString & /*client_id*/, const QStringList &/*channels*/);

protected:

    bool setPlaybackMixer(const QString &_mixer_id, const QString &Channel);


protected slots:

    void selectTempFile();
    void slotComboPlaybackMixerSelected(int idx);
    void updatePlaybackMixerChannelAlternatives();
    
    void slotOK();
    void slotCancel();
    void slotSetDirty();
    void slotUpdateConfig();

protected:

    bool    m_ignoreGUIChanges;
    int     m_myControlChange;

    typedef GUIListHelper<QComboBox, QString> StringListHelper;

    StringListHelper  m_PlaybackMixerHelper;
    StringListHelper  m_PlaybackChannelHelper;

    TimeShifter *m_Shifter;
    bool         m_dirty;
};

#endif
