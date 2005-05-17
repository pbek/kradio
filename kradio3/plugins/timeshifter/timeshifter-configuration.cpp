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

#include <kradio/libkradio/utils.h>
#include <kradio/libkradio-gui/gui_list_helper.h>
#include "timeshifter-configuration.h"
#include "timeshifter.h"

TimeShifterConfiguration::TimeShifterConfiguration (QWidget *parent, TimeShifter *shifter)
  : TimeShifterConfigurationUI(parent),
    m_ignoreGUIChanges(false),
    m_myControlChange(0),
    m_PlaybackMixerHelper(comboPlaybackMixerDevice, StringListHelper::SORT_BY_DESCR),
    m_PlaybackChannelHelper(comboPlaybackMixerChannel, IntListHelper::SORT_BY_ID),
    m_Shifter(shifter)
{
    QObject::connect(buttonSelectTempFile, SIGNAL(clicked()),
                     this, SLOT(selectTempFile()));
    QObject::connect(comboPlaybackMixerDevice, SIGNAL(activated(int)),
                     this, SLOT(slotComboPlaybackMixerSelected(int)));
    slotCancel();
}


TimeShifterConfiguration::~TimeShifterConfiguration ()
{
}


bool TimeShifterConfiguration::connectI (Interface *i)
{
    bool a = ISoundStreamClient::connectI(i);
    if (a) {
        getSoundStreamServer()->register4_notifyPlaybackChannelsChanged(this);
    }
    return a;
}


bool TimeShifterConfiguration::disconnectI (Interface *i)
{
    bool a = ISoundStreamClient::disconnectI(i);
    return a;
}


void TimeShifterConfiguration::noticeConnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid)
{
    if (i && pointer_valid && i->supportsPlayback()) {
        setPlaybackMixer(m_PlaybackMixerHelper.count()   ? m_PlaybackMixerHelper.getCurrentItem()   : QString::null,
                         m_PlaybackChannelHelper.count() ? m_PlaybackChannelHelper.getCurrentItem() : -1);
    }
}


void TimeShifterConfiguration::noticeDisconnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid)
{
    if (i && pointer_valid && i->supportsPlayback()) {
        setPlaybackMixer(m_PlaybackMixerHelper.count()   ? m_PlaybackMixerHelper.getCurrentItem()   : QString::null,
                         m_PlaybackChannelHelper.count() ? m_PlaybackChannelHelper.getCurrentItem() : -1);
    }
}



bool TimeShifterConfiguration::setPlaybackMixer(const QString &_mixer_id, int Channel)
{
    QString mixer_id = _mixer_id;
    bool old = m_ignoreGUIChanges;
    m_ignoreGUIChanges = true;

    m_PlaybackMixerHelper.setData(getPlaybackClientDescriptions());
    m_PlaybackMixerHelper.setCurrentItem(mixer_id);
    mixer_id = m_PlaybackMixerHelper.count() ? m_PlaybackMixerHelper.getCurrentItem() : QString::null;

    ISoundStreamClient *mixer = getSoundStreamClientWithID(mixer_id);
    if (mixer) {
        m_PlaybackChannelHelper.setData(mixer->getPlaybackChannels());
        m_PlaybackChannelHelper.setCurrentItem(Channel);
    }
    labelPlaybackMixerChannel->setEnabled(mixer != NULL);
    comboPlaybackMixerChannel->setEnabled(mixer != NULL);

    m_ignoreGUIChanges = old;
    return true;
}


// GUI Slots


void TimeShifterConfiguration::selectTempFile()
{
    KFileDialog fd("/tmp/", i18n("any ( * )"), this, i18n("TimeShifter Temporary File Selection"), TRUE);
    fd.setMode(KFile::File);
    fd.setCaption (i18n("Select TimeShifter Temporary File"));

    if (fd.exec() == QDialog::Accepted) {
        editTempFile->setText(fd.selectedFile());
    }
}


void TimeShifterConfiguration::slotComboPlaybackMixerSelected(int /*idx*/)
{
    if (m_ignoreGUIChanges) return;
    setPlaybackMixer(m_PlaybackMixerHelper.getCurrentItem(), m_PlaybackChannelHelper.getCurrentItem());
}


void TimeShifterConfiguration::slotOK()
{
    if (m_Shifter) {
        m_Shifter->setTempFile(editTempFile->text(), editTempFileSize->value() * (Q_UINT64)(1024 * 1024));
        m_Shifter->setPlaybackMixer(m_PlaybackMixerHelper.getCurrentItem(),
                                    m_PlaybackChannelHelper.getCurrentItem());
    }
}


void TimeShifterConfiguration::slotCancel()
{
    if (m_Shifter) {
        editTempFile->setText(m_Shifter->getTempFileName());
        editTempFileSize->setValue(m_Shifter->getTempFileMaxSize() / 1024 / 1024);

        setPlaybackMixer(m_Shifter->getPlaybackMixer(), m_Shifter->getPlaybackMixerChannel());
    }
}


bool TimeShifterConfiguration::noticePlaybackChannelsChanged(const QString & client_id, const QMap<int, QString> &/*channels*/)
{
    m_PlaybackMixerHelper.setData(getPlaybackClientDescriptions());
    if (m_PlaybackMixerHelper.getCurrentItem() == client_id) {
        setPlaybackMixer(client_id, m_PlaybackChannelHelper.count() ? m_PlaybackChannelHelper.getCurrentItem() : -1);
    }
    return true;
}



#include "timeshifter-configuration.moc"
