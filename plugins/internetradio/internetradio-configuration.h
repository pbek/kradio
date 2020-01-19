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

#include <QWidget>

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

    bool connectI    (Interface *i) override;
    bool disconnectI (Interface *i) override;

    void noticeConnectedSoundClient   (ISoundStreamClient::thisInterface *i, bool pointer_valid) override;
    void noticeDisconnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid) override;

// ISoundStreamClient

RECEIVERS:

    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid) override;

    bool noticePlaybackChannelsChanged(const QString & /*client_id*/, const QStringList &/*channels*/) override;
    bool noticeSoundStreamCreated(SoundStreamID /*id*/) override;


public slots:
    void slotNoticePlaybackMixerChanged(const QString &_mixer_id, const QString &Channel, bool muteOnPowerOff, bool force);
    void slotBufferSettingsChanged     (int inputBufSize, int outputBufSize);
    void slotWatchdogSettingsChanged   (int timeout);
    void slotDecoderSettingsChanged    (int probe_size, double analysis_time);

protected slots:

    void slotComboPlaybackMixerSelected(int idx);
    void updatePlaybackMixerChannelAlternatives();

    void slotOK();
    void slotCancel();

signals:

    void sigPlaybackMixerChanged   (const QString &soundStreamClientID, const QString &ch, bool muteOnPowerOff, bool force);
    void sigBufferSettingsChanged  (int inputBufferSize, int outputBufferSize);
    void sigWatchdogSettingsChanged(int watchdogTimeout);
    void sigDecoderSettingsChanged (int probe_size, double analysis_time);


protected:

    SoundStreamID     m_SoundStreamID;

    bool              m_ignoreGUIChanges;

    typedef GUIListHelper<QComboBox, QString>       StringListHelper;

    StringListHelper  m_PlaybackMixerHelper;
    StringListHelper  m_PlaybackChannelHelper;

    bool              m_orgMuteOnPowerOff;
    int               m_orgInputBufferSize;
    int               m_orgOutputBufferSize;
    int               m_orgWatchdogTimeout;
    int               m_orgProbeSize;
    double            m_orgAnalysisTime;
};

#endif
