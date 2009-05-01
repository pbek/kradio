/***************************************************************************
                          internetradio-configuration.h  -  description
                             -------------------
    begin                : Mon Feb 23 2009
    copyright            : (C) 2009 by Martin Witte
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

#ifndef KRADIO_INTERNETRADIO_CONFIGURATION_H
#define KRADIO_INTERNETRADIO_CONFIGURATION_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtGui/QWidget>

#include "soundstreamclient_interfaces.h"
#include "gui_list_helper.h"

#include "ui_internetradio-configuration-ui.h"

class QComboBox;

class InternetRadioConfiguration : public QWidget,
                                   public Ui_InternetRadioConfigurationUI,
                                   public ISoundStreamClient
{
Q_OBJECT
public :
    InternetRadioConfiguration (QWidget *parent, SoundStreamID id);
    ~InternetRadioConfiguration ();

    bool connectI (Interface *i);
    bool disconnectI (Interface *i);

    void noticeConnectedSoundClient   (ISoundStreamClient::thisInterface *i, bool pointer_valid);
    void noticeDisconnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid);

// ISoundStreamClient

RECEIVERS:
    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid);

    bool noticePlaybackChannelsChanged(const QString & /*client_id*/, const QStringList &/*channels*/);
    bool noticeSoundStreamCreated(SoundStreamID /*id*/);


protected slots:

    void slotComboPlaybackMixerSelected(int idx);

    void slotNoticePlaybackMixerChanged(const QString &_mixer_id, const QString &Channel, bool muteOnPowerOff, bool force);

    void slotOK();
    void slotCancel();

signals:

    void sigPlaybackMixerChanged(const QString &soundStreamClientID, const QString &ch, bool muteOnPowerOff, bool force);


protected:

    SoundStreamID m_SoundStreamID;

    bool    m_ignoreGUIChanges;

    int     m_myControlChange;

    typedef GUIListHelper<QComboBox, QString>       StringListHelper;
    typedef GUISimpleListHelper<QComboBox>          ChannelListHelper;

    StringListHelper  m_PlaybackMixerHelper;
    ChannelListHelper m_PlaybackChannelHelper;

    QString           m_orgMixerID;
    QString           m_orgChannelID;
    bool              m_orgMuteOnPowerOff;
};

#endif
