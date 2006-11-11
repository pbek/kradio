/***************************************************************************
                          v4lradio-configuration.cpp  -  description
                             -------------------
    begin                : Fre Jun 20 2003
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

#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/soundcard.h>

#include <qspinbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qfile.h>
#include <qpushbutton.h>

#include <kfiledialog.h>
#include <knuminput.h>
#include <klocale.h>

#include "../../src/include/utils.h"
#include "../../src/include/gui_list_helper.h"
#include "timeshifter-configuration.h"
#include "timeshifter.h"

TimeShifterConfiguration::TimeShifterConfiguration (QWidget *parent, TimeShifter *shifter)
  : TimeShifterConfigurationUI(parent),
    m_ignoreGUIChanges(false),
    m_myControlChange(0),
    m_PlaybackMixerHelper(comboPlaybackMixerDevice, StringListHelper::SORT_BY_DESCR),
    m_PlaybackChannelHelper(comboPlaybackMixerChannel),
    m_Shifter(shifter),
    m_dirty(true)
{
    QObject::connect(buttonSelectTempFile, SIGNAL(clicked()),
                     this, SLOT(selectTempFile()));
    QObject::connect(comboPlaybackMixerDevice, SIGNAL(activated(int)),
                     this, SLOT(slotComboPlaybackMixerSelected(int)));

    connect(editTempFile,              SIGNAL(textChanged(const QString&)), this, SLOT(slotSetDirty()));
    connect(editTempFileSize,          SIGNAL(valueChanged(int)),           this, SLOT(slotSetDirty()));
    connect(comboPlaybackMixerChannel, SIGNAL(activated( int )),            this, SLOT(slotSetDirty()));
    connect(comboPlaybackMixerDevice,  SIGNAL(activated( int )),            this, SLOT(slotSetDirty()));
    slotCancel();
}


TimeShifterConfiguration::~TimeShifterConfiguration ()
{
}


bool TimeShifterConfiguration::connectI (Interface *i)
{
    bool a = ISoundStreamClient::connectI(i);
    return a;
}


bool TimeShifterConfiguration::disconnectI (Interface *i)
{
    bool a = ISoundStreamClient::disconnectI(i);
    return a;
}

void TimeShifterConfiguration::noticeConnectedI (ISoundStreamServer *s, bool pointer_valid)
{
    ISoundStreamClient::noticeConnectedI(s, pointer_valid);
    if (s && pointer_valid) {
        s->register4_notifyPlaybackChannelsChanged(this);
    }
}

void TimeShifterConfiguration::noticeConnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid)
{
    if (i && pointer_valid && i->supportsPlayback() && m_Shifter) {
        const QString &org_mid     = m_Shifter->getPlaybackMixer();
        bool           org_present = m_PlaybackMixerHelper.contains(org_mid);
        const QString &mid         = org_present ? m_PlaybackMixerHelper.getCurrentItem() : org_mid;
        const QString &org_ch      = m_Shifter->getPlaybackMixerChannel();
        const QString &ch          = org_present ? m_PlaybackChannelHelper.getCurrentText() : org_ch;
        setPlaybackMixer(mid, ch);
    }
}


void TimeShifterConfiguration::noticeDisconnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid)
{
    if (i && pointer_valid && i->supportsPlayback()) {
        setPlaybackMixer(m_Shifter->getPlaybackMixer(), m_Shifter->getPlaybackMixerChannel());
    }
}



bool TimeShifterConfiguration::setPlaybackMixer(const QString &_mixer_id, const QString &Channel)
{
    QString mixer_id = _mixer_id;
    bool old = m_ignoreGUIChanges;
    m_ignoreGUIChanges = true;

    m_PlaybackMixerHelper.setData(getPlaybackClientDescriptions());
    m_PlaybackMixerHelper.setCurrentItem(mixer_id);
    mixer_id = m_PlaybackMixerHelper.getCurrentItem();

    ISoundStreamClient *mixer = getSoundStreamClientWithID(mixer_id);
    if (mixer) {
        m_PlaybackChannelHelper.setData(mixer->getPlaybackChannels());
        m_PlaybackChannelHelper.setCurrentText(m_PlaybackChannelHelper.contains(Channel) ? Channel : m_Shifter->getPlaybackMixerChannel());
    }
    labelPlaybackMixerChannel->setEnabled(mixer != NULL);
    comboPlaybackMixerChannel->setEnabled(mixer != NULL);

    m_ignoreGUIChanges = old;
    return true;
}


// GUI Slots


void TimeShifterConfiguration::selectTempFile()
{
    KFileDialog fd("/tmp/",
                   i18n("any ( * )").ascii(),
                   this,
                   i18n("TimeShifter Temporary File Selection").ascii(),
                   TRUE);
    fd.setMode(KFile::File);
    fd.setCaption (i18n("Select TimeShifter Temporary File"));

    if (fd.exec() == QDialog::Accepted) {
        editTempFile->setText(fd.selectedFile());
    }
}


void TimeShifterConfiguration::slotComboPlaybackMixerSelected(int /*idx*/)
{
    if (m_ignoreGUIChanges) return;
    setPlaybackMixer(m_PlaybackMixerHelper.getCurrentItem(), m_PlaybackChannelHelper.getCurrentText());
}


void TimeShifterConfiguration::slotOK()
{
    if (m_Shifter && m_dirty) {
        m_Shifter->setTempFile(editTempFile->text(), editTempFileSize->value() * (Q_UINT64)(1024 * 1024));
        m_Shifter->setPlaybackMixer(m_PlaybackMixerHelper.getCurrentItem(),
                                    m_PlaybackChannelHelper.getCurrentText());
        m_dirty = false;
    }
}


void TimeShifterConfiguration::slotCancel()
{
    if (m_Shifter && m_dirty) {
        editTempFile->setText(m_Shifter->getTempFileName());
        editTempFileSize->setValue(m_Shifter->getTempFileMaxSize() / 1024 / 1024);

        setPlaybackMixer(m_Shifter->getPlaybackMixer(), m_Shifter->getPlaybackMixerChannel());
        m_dirty = false;
    }
}


bool TimeShifterConfiguration::noticePlaybackChannelsChanged(const QString & client_id, const QStringList &/*channels*/)
{
    if (m_PlaybackMixerHelper.getCurrentItem() == client_id) {
        setPlaybackMixer(client_id, m_PlaybackChannelHelper.getCurrentText());
    }
    return true;
}


void TimeShifterConfiguration::slotSetDirty()
{
    if (!m_ignoreGUIChanges) {
        m_dirty = true;
    }
}

void TimeShifterConfiguration::slotUpdateConfig()
{
    slotSetDirty();
    slotCancel();
}

#include "timeshifter-configuration.moc"
