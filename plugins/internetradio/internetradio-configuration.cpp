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
#include <linux/soundcard.h>

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

InternetRadioConfiguration::InternetRadioConfiguration (QWidget *parent, SoundStreamID ssid)
  : QWidget(parent),
    m_SoundStreamID(ssid),
    m_ignoreGUIChanges(false),
    m_myControlChange(0),
    m_PlaybackMixerHelper  (NULL, StringListHelper::SORT_BY_DESCR),
    m_PlaybackChannelHelper(NULL)
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
        const QString &org_mid     = m_orgMixerID;
        bool           org_present = m_PlaybackMixerHelper.contains(org_mid);
        const QString &mid         = org_present ? m_PlaybackMixerHelper.getCurrentItem() : org_mid;
        const QString &org_ch      = m_orgChannelID;
        const QString &ch          = org_present ? m_PlaybackChannelHelper.getCurrentText() : org_ch;
        slotNoticePlaybackMixerChanged(mid, ch, m_orgMuteOnPowerOff, /* force = */ false);
    }
}


void InternetRadioConfiguration::noticeDisconnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid)
{
    if (i && pointer_valid && i->supportsPlayback()) {
        slotNoticePlaybackMixerChanged(m_orgMixerID, m_orgChannelID, m_orgMuteOnPowerOff, /* force = */ false);
    }
}


void InternetRadioConfiguration::slotNoticePlaybackMixerChanged(const QString &_mixer_id, const QString &Channel, bool muteOnPowerOff, bool /*force*/)
{
    QString mixer_id = _mixer_id;
    bool old = m_ignoreGUIChanges;
    m_ignoreGUIChanges = true;

    m_PlaybackMixerHelper.setData(getPlaybackClientDescriptions());
    m_PlaybackMixerHelper.setCurrentItem(mixer_id);
    ISoundStreamClient *mixer = NULL;
    if (m_PlaybackMixerHelper.count()) {
        mixer_id = m_PlaybackMixerHelper.getCurrentItem();
        mixer = getSoundStreamClientWithID(mixer_id);
        if (mixer) {
            m_PlaybackChannelHelper.setData(mixer->getPlaybackChannels());
            m_PlaybackChannelHelper.setCurrentText(m_PlaybackChannelHelper.contains(Channel) ? Channel : m_orgChannelID);
        }
    }
    labelPlaybackMixerChannel->setEnabled(mixer != NULL);
    comboPlaybackMixerChannel->setEnabled(mixer != NULL);

    m_orgMixerID        = _mixer_id;
    m_orgChannelID      = Channel;
    m_orgMuteOnPowerOff = muteOnPowerOff;

    m_ignoreGUIChanges = old;
}


bool InternetRadioConfiguration::noticeSoundStreamCreated(SoundStreamID id)
{
    if (id.HasSamePhysicalID(m_SoundStreamID)) {
        m_SoundStreamID = id;
    }
    return true;
}


// GUI Slots


void InternetRadioConfiguration::slotComboPlaybackMixerSelected(int /*idx*/)
{
    if (m_ignoreGUIChanges) return;
    QString id = m_PlaybackMixerHelper.getCurrentItem();
    slotNoticePlaybackMixerChanged(id, m_orgChannelID, m_orgMuteOnPowerOff, /* force = */ false);
}


void InternetRadioConfiguration::slotOK()
{
    QString mixer_id;
    QString channel_id;
    if (m_PlaybackMixerHelper.count()) {
        mixer_id = m_PlaybackMixerHelper.getCurrentItem();
    }
    if (m_PlaybackChannelHelper.count()) {
        channel_id = m_PlaybackChannelHelper.getCurrentText();
    }
    bool muteOnPowerOff = cbMutePlaybackMixerOnPowerOff->isChecked();
    emit sigPlaybackMixerChanged (mixer_id, channel_id, muteOnPowerOff, /* force = */ false);
}



void InternetRadioConfiguration::slotCancel()
{
    slotNoticePlaybackMixerChanged(m_orgMixerID, m_orgChannelID, m_orgMuteOnPowerOff, /* force = */ false);
}


bool InternetRadioConfiguration::noticePlaybackChannelsChanged(const QString & client_id, const QStringList &/*channels*/)
{
    if (m_PlaybackMixerHelper.count() && m_PlaybackMixerHelper.getCurrentItem() == client_id) {
        slotNoticePlaybackMixerChanged(client_id, m_PlaybackChannelHelper.getCurrentText(), m_orgMuteOnPowerOff, /* force = */ false);
    }
    return true;
}



#include "internetradio-configuration.moc"
