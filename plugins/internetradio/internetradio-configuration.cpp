/***************************************************************************
                          internetradio-configuration.cpp  -  description
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

#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <QtGui/QSpinBox>
#include <QtGui/QLineEdit>
#include <QtGui/QLabel>
#include <QtCore/QFile>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QCheckBox>

#include <kfiledialog.h>
#include <knuminput.h>
#include <klocale.h>
#include <ktabwidget.h>

#include "utils.h"
#include "gui_list_helper.h"
#include "internetradio-configuration.h"

#include "errorlog_interfaces.h"

InternetRadioConfiguration::InternetRadioConfiguration (QWidget *parent, SoundStreamID ssid)
  : QWidget(parent),
    m_SoundStreamID(ssid),
    m_ignoreGUIChanges(false),
    m_PlaybackMixerHelper  (NULL, StringListHelper::SORT_BY_DESCR),
    m_PlaybackChannelHelper(NULL, StringListHelper::SORT_NONE),
    m_orgInputBufferSize (128*1024),
    m_orgOutputBufferSize(512*1024),
    m_orgWatchdogTimeout (0),
    m_orgProbeSize       (8192),
    m_orgAnalysisTime    (0.8)
{

    setupUi(this);
    m_PlaybackMixerHelper  .setList(comboPlaybackMixerDevice);
    m_PlaybackChannelHelper.setList(comboPlaybackMixerChannel);

    QObject::connect(comboPlaybackMixerDevice, SIGNAL(activated(int)),
                     this, SLOT(slotComboPlaybackMixerSelected(int)));

}


InternetRadioConfiguration::~InternetRadioConfiguration ()
{
}


bool InternetRadioConfiguration::connectI (Interface *i)
{
    bool a = ISoundStreamClient::connectI(i);
    return a;
}


bool InternetRadioConfiguration::disconnectI (Interface *i)
{
    bool a = ISoundStreamClient::disconnectI(i);
    return a;
}

void InternetRadioConfiguration::noticeConnectedI (ISoundStreamServer *s, bool pointer_valid)
{
    ISoundStreamClient::noticeConnectedI(s, pointer_valid);
    if (s && pointer_valid) {
        s->register4_notifyPlaybackChannelsChanged(this);
        s->register4_notifySoundStreamCreated(this);
    }
}

void InternetRadioConfiguration::noticeConnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid)
{
    if (i && pointer_valid && i->supportsPlayback()) {
        updatePlaybackMixerChannelAlternatives();
    }
}


void InternetRadioConfiguration::noticeDisconnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid)
{
    if (i && pointer_valid && i->supportsPlayback()) {
        updatePlaybackMixerChannelAlternatives();
    }
}


bool InternetRadioConfiguration::noticePlaybackChannelsChanged(const QString & /*client_id*/, const QStringList &/*channels*/)
{
    updatePlaybackMixerChannelAlternatives();
    return true;
}

void InternetRadioConfiguration::updatePlaybackMixerChannelAlternatives()
{
    m_PlaybackMixerHelper.alternativesChanged(getPlaybackClientDescriptions());
    ISoundStreamClient *mixer = getSoundStreamClientWithID(m_PlaybackMixerHelper.getCurrentItemID());
    if (mixer) {
        m_PlaybackChannelHelper.alternativesChanged(mixer->getPlaybackChannels());
    }
    labelPlaybackMixerChannel->setEnabled(mixer != NULL);
    comboPlaybackMixerChannel->setEnabled(mixer != NULL);
}


bool InternetRadioConfiguration::noticeSoundStreamCreated(SoundStreamID id)
{
    if (id.HasSamePhysicalID(m_SoundStreamID)) {
        m_SoundStreamID = id;
    }
    return true;
}


// from InternetRadio instance

void InternetRadioConfiguration::slotNoticePlaybackMixerChanged(const QString &mixer_id, const QString &Channel, bool muteOnPowerOff, bool /*force*/)
{
//     IErrorLogClient::staticLogDebug(QString("mixer: %1, channel: %2").arg(mixer_id).arg(Channel));
    m_PlaybackMixerHelper  .setOrgItemID(mixer_id);
    m_PlaybackChannelHelper.setOrgItemID(Channel);
    cbMutePlaybackMixerOnPowerOff->setChecked(m_orgMuteOnPowerOff = muteOnPowerOff);
}


void InternetRadioConfiguration::slotBufferSettingsChanged(int inputBufSize, int outputBufSize)
{
    m_orgInputBufferSize  = inputBufSize;
    m_orgOutputBufferSize = outputBufSize;
    spinboxStreamInputBufferSize ->setValue(m_orgInputBufferSize  / 1024);
    spinboxStreamOutputBufferSize->setValue(m_orgOutputBufferSize / 1024);
}


void InternetRadioConfiguration::slotWatchdogSettingsChanged(int timeout)
{
    m_orgWatchdogTimeout = timeout;
    spinboxWatchdogTimeout->setValue(m_orgWatchdogTimeout);
}

void InternetRadioConfiguration::slotDecoderSettingsChanged(int probe_size, double analysis_time)
{
    m_orgProbeSize    = probe_size;
    m_orgAnalysisTime = analysis_time;
    spinboxProbeSize   ->setValue(m_orgProbeSize / 1024);
    spinboxAnalysisTime->setValue(m_orgAnalysisTime);
}

// GUI Slots


void InternetRadioConfiguration::slotComboPlaybackMixerSelected(int /*idx*/)
{
    if (m_ignoreGUIChanges) return;
    // since we do not know in which order signals are delivered, ensure that the helper already nows about the selection change
    m_PlaybackMixerHelper.slotUserSelection();
    updatePlaybackMixerChannelAlternatives();
}


void InternetRadioConfiguration::slotOK()
{
    m_PlaybackMixerHelper  .slotOK();
    m_PlaybackChannelHelper.slotOK();

    QString mixer_id       = m_PlaybackMixerHelper  .getCurrentItemID();
    QString channel_id     = m_PlaybackChannelHelper.getCurrentItemID();
    m_orgMuteOnPowerOff    = cbMutePlaybackMixerOnPowerOff->isChecked();
    m_orgInputBufferSize   = spinboxStreamInputBufferSize ->value() * 1024;
    m_orgOutputBufferSize  = spinboxStreamOutputBufferSize->value() * 1024;
    m_orgWatchdogTimeout   = spinboxWatchdogTimeout       ->value();
    m_orgProbeSize         = spinboxProbeSize             ->value() * 1024;
    m_orgAnalysisTime      = spinboxAnalysisTime          ->value();
    
    emit sigPlaybackMixerChanged   (mixer_id, channel_id, m_orgMuteOnPowerOff, /* force = */ false);
    emit sigBufferSettingsChanged  (m_orgInputBufferSize, m_orgOutputBufferSize);
    emit sigWatchdogSettingsChanged(m_orgWatchdogTimeout);
    emit sigDecoderSettingsChanged (m_orgProbeSize, m_orgAnalysisTime);
}



void InternetRadioConfiguration::slotCancel()
{
    m_PlaybackMixerHelper  .slotCancel();
    m_PlaybackChannelHelper.slotCancel();
    cbMutePlaybackMixerOnPowerOff->setChecked(m_orgMuteOnPowerOff);
    spinboxStreamInputBufferSize ->setValue(m_orgInputBufferSize  / 1024);
    spinboxStreamOutputBufferSize->setValue(m_orgOutputBufferSize / 1024);
    spinboxWatchdogTimeout       ->setValue(m_orgWatchdogTimeout);
    spinboxProbeSize             ->setValue(m_orgProbeSize        / 1024);
    spinboxAnalysisTime          ->setValue(m_orgAnalysisTime);
}




#include "internetradio-configuration.moc"
