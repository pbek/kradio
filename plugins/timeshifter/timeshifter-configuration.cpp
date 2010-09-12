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

#include <QtGui/QSpinBox>
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtCore/QFile>

#include <kfiledialog.h>
#include <knuminput.h>
#include <klocale.h>

#include "utils.h"
#include "gui_list_helper.h"
#include "timeshifter-configuration.h"
#include "timeshifter.h"

TimeShifterConfiguration::TimeShifterConfiguration (QWidget *parent, TimeShifter *shifter)
  : QWidget(parent),
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

    buttonSelectTempFile->setIcon(KIcon("document-open"));

    QObject::connect(buttonSelectTempFile, SIGNAL(clicked()),
                     this, SLOT(selectTempFile()));
    QObject::connect(comboPlaybackMixerDevice, SIGNAL(activated(int)),
                     this, SLOT(slotComboPlaybackMixerSelected(int)));

    connect(editTempFile,              SIGNAL(textChanged(const QString&)), this, SLOT(slotSetDirty()));
    connect(editTempFileSize,          SIGNAL(valueChanged(int)),           this, SLOT(slotSetDirty()));
    connect(&m_PlaybackMixerHelper,    SIGNAL(sigDirtyChanged(bool)),       this, SLOT(slotSetDirty()));
    connect(&m_PlaybackChannelHelper,  SIGNAL(sigDirtyChanged(bool)),       this, SLOT(slotSetDirty()));
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
    KFileDialog fd(KUrl("/tmp/"),
                   i18n("any ( * )"),
                   this);
    fd.setModal(true);
    fd.setMode(KFile::File);
    fd.setCaption (i18n("Select TimeShifter Temporary File"));

    if (fd.exec() == QDialog::Accepted) {
        editTempFile->setText(fd.selectedFile());
    }
}


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
                                    m_PlaybackChannelHelper.getCurrentItemID());
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

#include "timeshifter-configuration.moc"
