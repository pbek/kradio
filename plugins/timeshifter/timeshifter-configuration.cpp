/***************************************************************************
                          v4lradio-configuration.cpp  -  description
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

#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QFile>

#include <QtWidgets/QFileDialog>

#include <klocalizedstring.h>
#include <QtGui/QIcon>

#include "gui_list_helper.h"
#include "timeshifter-configuration.h"
#include "timeshifter.h"

TimeShifterConfiguration::TimeShifterConfiguration (QWidget *parent, TimeShifter *shifter)
  : PluginConfigPageBase(parent),
    m_ignoreGUIChanges(false),
    m_myControlChange(0),
    m_PlaybackMixerHelper  (NULL, StringListHelper::SORT_BY_DESCR),
    m_PlaybackChannelHelper(NULL, StringListHelper::SORT_NONE),
    m_Shifter(shifter),
    m_dirty(true)
{
    setupUi(this);
    m_PlaybackMixerHelper  .setList(comboPlaybackMixerDevice );
    m_PlaybackChannelHelper.setList(comboPlaybackMixerChannel);

    buttonSelectTempFile->setIcon(QIcon::fromTheme("document-open"));

    QObject::connect(buttonSelectTempFile,     &QPushButton::clicked,                       this, &TimeShifterConfiguration::selectTempFile);
    QObject::connect(comboPlaybackMixerDevice, QOverload<int>::of(&QComboBox::activated),   this, &TimeShifterConfiguration::slotComboPlaybackMixerSelected);
    QObject::connect(editTempFile,             &QLineEdit::textChanged,                     this, &TimeShifterConfiguration::slotSetDirty);
    QObject::connect(editTempFileSize,         QOverload<int>::of(&QSpinBox::valueChanged), this, &TimeShifterConfiguration::slotSetDirty);
    QObject::connect(&m_PlaybackMixerHelper,   &StringListHelper::sigDirtyChanged,          this, &TimeShifterConfiguration::slotSetDirty);
    QObject::connect(&m_PlaybackChannelHelper, &StringListHelper::sigDirtyChanged,          this, &TimeShifterConfiguration::slotSetDirty);
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
        updatePlaybackMixerChannelAlternatives();
    }
}

void TimeShifterConfiguration::noticeDisconnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid)
{
    if (i && pointer_valid && i->supportsPlayback()) {
        updatePlaybackMixerChannelAlternatives();
    }
}


void TimeShifterConfiguration::updatePlaybackMixerChannelAlternatives()
{
    m_PlaybackMixerHelper.alternativesChanged(getPlaybackClientDescriptions());
    ISoundStreamClient *mixer = getSoundStreamClientWithID(m_PlaybackMixerHelper.getCurrentItemID());
    if (mixer) {
        m_PlaybackChannelHelper.alternativesChanged(mixer->getPlaybackChannels());
    }
    labelPlaybackMixerChannel->setEnabled(mixer != NULL);
    comboPlaybackMixerChannel->setEnabled(mixer != NULL);
}



bool TimeShifterConfiguration::noticePlaybackChannelsChanged(const QString & /*client_id*/, const QStringList &/*channels*/)
{
    updatePlaybackMixerChannelAlternatives();
    return true;
}


bool TimeShifterConfiguration::setPlaybackMixer(const QString &mixer_id, const QString &Channel)
{
//     bool old = m_ignoreGUIChanges;
//     m_ignoreGUIChanges = true;

    m_PlaybackMixerHelper  .setOrgItemID(mixer_id);
    m_PlaybackChannelHelper.setOrgItemID(Channel);

//     m_ignoreGUIChanges = old;
    return true;
}


// GUI Slots


void TimeShifterConfiguration::selectTempFile()
{
    QFileDialog fd(this, 
                   i18n("Select TimeShifter Temporary File"),
                   "/tmp/",
                   i18n("any ( * )")
                  );
    fd.setModal(true);
    fd.setFileMode(QFileDialog::AnyFile);

    if (fd.exec() == QDialog::Accepted) {
        const QStringList flist = fd.selectedFiles();
        if (flist.size() >= 1) {
            editTempFile->setText(flist[0]);
        } // if list len >= 1
    } // if accepted
} // TimeShifterConfiguration::selectTempFile


void TimeShifterConfiguration::slotComboPlaybackMixerSelected(int /*idx*/)
{
    if (m_ignoreGUIChanges) return;
    // since we do not know in which order signals are delivered, ensure that the helper already nows about the selection change
    m_PlaybackMixerHelper.slotUserSelection();
    updatePlaybackMixerChannelAlternatives();
}


void TimeShifterConfiguration::slotOK()
{
    if (m_Shifter && m_dirty) {
        m_PlaybackMixerHelper  .slotOK();
        m_PlaybackChannelHelper.slotOK();
        m_Shifter->setTempFile(editTempFile->text(), editTempFileSize->value() * (quint64)(1024 * 1024));
        m_Shifter->setPlaybackMixer(m_PlaybackMixerHelper  .getCurrentItemID(),
                                    m_PlaybackChannelHelper.getCurrentItemID(), false);
        m_dirty = false;
    }
}


void TimeShifterConfiguration::slotCancel()
{
    if (m_Shifter && m_dirty) {
        setPlaybackMixer(m_Shifter->getPlaybackMixer(), m_Shifter->getPlaybackMixerChannel());

        m_PlaybackMixerHelper  .slotCancel();
        m_PlaybackChannelHelper.slotCancel();

        editTempFile    ->setText(m_Shifter->getTempFileName());
        editTempFileSize->setValue(m_Shifter->getTempFileMaxSize() / 1024 / 1024);

        m_dirty = false;
    }
}


void TimeShifterConfiguration::slotSetDirty()
{
    m_dirty = true;
}

void TimeShifterConfiguration::slotUpdateConfig()
{
    slotSetDirty();
    slotCancel();
}

