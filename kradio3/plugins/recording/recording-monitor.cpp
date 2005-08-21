/***************************************************************************
                          recording-monitor.cpp  -  description
                             -------------------
    begin                : Mo Sep 1 2003
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

#include "recording-monitor.h"
#include "recording-datamonitor.h"
#include "../../src/libkradio-gui/aboutwidget.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <kcombobox.h>

#include <klocale.h>
#include <kconfig.h>
#include <kaboutdata.h>

RecordingMonitor::RecordingMonitor(const QString &name)
  : QWidget(NULL, name.ascii()),
    WidgetPluginBase(name, i18n("Recording Monitor")),
    m_recording(false),
    m_defaultStreamDescription(QString::null)
{
    setCaption(i18n("Recording Monitor"));

    QVBoxLayout *l = new QVBoxLayout(this, 10, 4);
    QGridLayout *l0 = new QGridLayout(l, 6, 2);

    l0->addWidget(                      new QLabel(i18n("SoundStream"),    this), 0, 0);
    l0->addWidget(m_comboSoundStreamSelector = new KComboBox(              this), 0, 1);
    l0->addWidget(                      new QLabel(i18n("Status"),         this), 1, 0);
    l0->addWidget(m_labelStatus       = new QLabel(i18n("<undefined>"),    this), 1, 1);
    l0->addWidget(                      new QLabel(i18n("Recording File"), this), 2, 0);
    l0->addWidget(m_labelFileName     = new QLabel(i18n("<undefined>"),    this), 2, 1);
    l0->addWidget(                      new QLabel(i18n("File Size"),      this), 3, 0);
    l0->addWidget(m_labelSize         = new QLabel(i18n("<undefined>"),    this), 3, 1);
    l0->addWidget(                      new QLabel(i18n("Recording Time"), this), 4, 0);
    l0->addWidget(m_labelTime         = new QLabel(i18n("<undefined>"),    this), 4, 1);
    l0->addWidget(                      new QLabel(i18n("Sample Rate"),    this), 5, 0);
    l0->addWidget(m_labelRate         = new QLabel(i18n("<undefined>"),    this), 5, 1);

    QPushButton *close     = new QPushButton(i18n("&Close"), this);
    m_btnStartStop         = new QPushButton(i18n("&Record"), this);
    QObject::connect(close, SIGNAL(clicked()), this, SLOT(hide()));
    QObject::connect(m_btnStartStop, SIGNAL(clicked()), this, SLOT(slotStartStopRecording()));

    m_dataMonitor = new RecordingDataMonitor(this, NULL);
    m_dataMonitor->setEnabled(false);

    QHBoxLayout *hl0 = new QHBoxLayout(l);
    hl0->addWidget(m_dataMonitor);

    QHBoxLayout *hl2 = new QHBoxLayout(l);
    hl2->addItem(new QSpacerItem(10, 1));
    hl2->addWidget(close);
    hl2->addWidget(m_btnStartStop);
    hl2->addItem(new QSpacerItem(10, 1));


    m_comboSoundStreamSelector->insertItem(i18n("nothing"));
    QObject::connect(m_comboSoundStreamSelector, SIGNAL(activated(int)), this, SLOT(slotStreamSelected(int)));

    updateRecordingButton();
}


RecordingMonitor::~RecordingMonitor()
{
}

// WidgetPluginBase

void   RecordingMonitor::saveState (KConfig *config) const
{
    config->setGroup(QString("recordingmonitor-") + name());

    WidgetPluginBase::saveState(config);
}


void   RecordingMonitor::restoreState (KConfig *config)
{
    config->setGroup(QString("recordingmonitor-") + name());

    WidgetPluginBase::restoreState(config, false);
}


bool   RecordingMonitor::connectI(Interface *i)
{
    bool a = ISoundStreamClient::connectI(i);
    bool b = WidgetPluginBase::connectI(i);
    return a || b;
}

bool   RecordingMonitor::disconnectI(Interface *i)
{
    bool a = ISoundStreamClient::disconnectI(i);
    bool b = WidgetPluginBase::disconnectI(i);
    if (a) {
        m_comboSoundStreamSelector->clear();
        m_SoundStreamID2idx.clear();
        m_idx2SoundStreamID.clear();
        m_comboSoundStreamSelector->insertItem(i18n("nothing"));
    }
    return a || b;
}


void RecordingMonitor::noticeConnectedI (ISoundStreamServer *s, bool pointer_valid)
{
    ISoundStreamClient::noticeConnectedI(s, pointer_valid);
    if (s && pointer_valid) {
        s->register4_notifySoundStreamCreated(this);
        s->register4_notifySoundStreamClosed(this);
        s->register4_notifySoundStreamChanged(this);
        s->register4_notifySoundStreamData(this);
        s->register4_sendStartRecordingWithFormat(this);
        s->register4_sendStopRecording(this);

        QMap<QString, SoundStreamID> tmp;
        queryEnumerateSoundStreams(tmp);

        m_comboSoundStreamSelector->clear();
        m_SoundStreamID2idx.clear();
        m_idx2SoundStreamID.clear();
        m_comboSoundStreamSelector->insertItem(i18n("nothing"));
        QMapConstIterator<QString, SoundStreamID> end = tmp.end();
        for (QMapConstIterator<QString, SoundStreamID> it = tmp.begin(); it != end; ++it) {
            int idx = m_comboSoundStreamSelector->count();
            m_comboSoundStreamSelector->insertItem(it.key());
            m_idx2SoundStreamID[idx] = *it;
            m_SoundStreamID2idx[*it] = idx;
        }
    }
}

ConfigPageInfo  RecordingMonitor::createConfigurationPage()
{
    return ConfigPageInfo();
}

AboutPageInfo   RecordingMonitor::createAboutPage()
{
/*    KAboutData aboutData("kradio",
                         NULL,
                         NULL,
                         I18N_NOOP("Recording Monitor Plugin for KRadio"),
                         KAboutData::License_GPL,
                         "(c) 2002-2005 Martin Witte",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  "", "witte@kawo1.rwth-aachen.de");

    return AboutPageInfo(
              new KRadioAboutWidget(aboutData, KRadioAboutWidget::AbtTabbed),
              i18n("Recording Monitor"),
              i18n("Recording Monitor Plugin"),
              "goto"
           );
*/
    return AboutPageInfo();
}


void RecordingMonitor::show()
{
    WidgetPluginBase::pShow();
    QWidget::show();
}


void RecordingMonitor::hide()
{
    WidgetPluginBase::pHide();
    QWidget::hide();
}


void RecordingMonitor::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    WidgetPluginBase::pShowEvent(e);
    m_comboSoundStreamSelector->setCurrentItem(1);
    slotStreamSelected(1);
}


void RecordingMonitor::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);
    WidgetPluginBase::pHideEvent(e);
    m_comboSoundStreamSelector->setCurrentItem(0);
    slotStreamSelected(0);
}


void RecordingMonitor::slotStartStopRecording()
{
    if (m_currentStream.isValid()) {
        if (m_recording) {
            sendStopRecording(m_currentStream);
        } else {
            sendStartRecording(m_currentStream);
        }
    }
    updateRecordingButton();
}


bool RecordingMonitor::noticeSoundStreamCreated(SoundStreamID id)
{
    QString tmp = QString::null;
    querySoundStreamDescription(id, tmp);

    int idx = m_comboSoundStreamSelector->count();
    m_comboSoundStreamSelector->insertItem(tmp);
    m_idx2SoundStreamID[idx] = id;
    m_SoundStreamID2idx[id]  = idx;

    if (tmp == m_defaultStreamDescription) {
        m_comboSoundStreamSelector->setCurrentItem(idx);
        slotStreamSelected(idx);
    }
    return true;
}


bool RecordingMonitor::noticeSoundStreamClosed(SoundStreamID id)
{
    if (m_SoundStreamID2idx.contains(id)) {
        int idx = m_SoundStreamID2idx[id];
        m_idx2SoundStreamID.clear();
        m_SoundStreamID2idx.remove(id);
        QMapIterator<SoundStreamID, int> end = m_SoundStreamID2idx.end();
        for (QMapIterator<SoundStreamID, int> it = m_SoundStreamID2idx.begin(); it != end; ++it) {
            if (*it > idx) {
                (*it)--;
            }
            m_idx2SoundStreamID[*it] = it.key();
        }
        m_comboSoundStreamSelector->removeItem(idx);
        slotStreamSelected(m_comboSoundStreamSelector->currentItem());
        return true;
    }
    return false;
}


bool RecordingMonitor::noticeSoundStreamChanged(SoundStreamID id)
{
    if (m_SoundStreamID2idx.contains(id)) {
        int idx = m_SoundStreamID2idx[id];
        QString tmp = QString::null;
        querySoundStreamDescription(id, tmp);
        m_comboSoundStreamSelector->changeItem(tmp, idx);
        if (idx == m_comboSoundStreamSelector->currentItem()) {
            m_defaultStreamDescription = tmp;
        }
        return true;
    }
    return false;
}

bool RecordingMonitor::startRecordingWithFormat(SoundStreamID id, const SoundFormat &/*sf*/, SoundFormat &/*real_format*/)
{
    if (id == m_currentStream) {
        m_recording = true;
        updateRecordingButton();
    }
    return false;
}

bool RecordingMonitor::stopRecording(SoundStreamID id)
{
    if (id == m_currentStream) {
        m_recording = false;
        updateRecordingButton();
    }
    return false;
}

bool RecordingMonitor::noticeSoundStreamData(SoundStreamID id,
    const SoundFormat &sf, const char *data, size_t size,
    const SoundMetaData &md
)
{
    if (m_idx2SoundStreamID[m_comboSoundStreamSelector->currentItem()] == id) {

        m_labelFileName->setText(md.url().url());

        double B = (double)md.position() + (double)size;

        double s = md.relativeTimestamp();

        int m = (int)(s / 60);   s -= 60 * m;
        int h = m / 60;   m %= 60;
        int d = h / 24;   h %= 24;
        QString time;
        if (d) {
            time.sprintf("%dd - %02d:%02d:%05.2f", d, h, m, s);
        } else {
            time.sprintf("%02d:%02d:%05.2f", h, m, s);
        }
        m_labelTime->setText(time);

        if (sf.m_Encoding == "raw") {
            m_dataMonitor->setEnabled(true);
            m_dataMonitor->noticeSoundStreamData(id, sf, data, size, md);
        } else {
            m_dataMonitor->setEnabled(false);
        }

        double kB = B / 1024;
        double MB = kB / 1024;
        double GB = MB / 1024;
        QString str_size;
        str_size.sprintf("%.0f Byte", B);
        if (kB > 1) str_size.sprintf("%.3f kB", kB);
        if (MB > 1) str_size.sprintf("%.3f MB", MB);
        if (GB > 1) str_size.sprintf("%.3f GB", GB);
        m_labelSize->setText(str_size);

        m_labelRate->setText(QString::number(sf.m_SampleRate) + " Hz");
        return true;
    }
    return false;
}


void RecordingMonitor::slotStreamSelected(int idx)
{
    SoundStreamID old_id = m_currentStream;
    if (old_id.isValid()) {
        sendStopCapture(old_id);
    }

    SoundStreamID id = m_idx2SoundStreamID.contains(idx) ? m_idx2SoundStreamID[idx] : SoundStreamID::InvalidID;
    if (id.isValid()) {

        m_defaultStreamDescription = m_comboSoundStreamSelector->text(idx);

        SoundFormat sf;
        sendStartCaptureWithFormat(id, sf, sf);
        m_dataMonitor   ->setEnabled(true);
        m_labelSize     ->setEnabled(true);
        m_labelSize     ->setEnabled(true);
        m_labelTime     ->setEnabled(true);
        m_labelRate     ->setEnabled(true);
        m_labelFileName ->setEnabled(true);
        m_labelStatus   ->setEnabled(true);
    } else {
        m_dataMonitor   ->setEnabled(false);
        m_labelSize     ->setEnabled(false);
        m_labelSize     ->setEnabled(false);
        m_labelTime     ->setEnabled(false);
        m_labelRate     ->setEnabled(false);
        m_labelFileName ->setEnabled(false);
        m_labelStatus   ->setEnabled(false);
    }
    m_currentStream = id;
    m_recording = false;
    SoundFormat sf;
    queryIsRecordingRunning(m_currentStream, m_recording, sf);
    updateRecordingButton();
}


void RecordingMonitor::updateRecordingButton()
{
    if (m_currentStream.isValid()) {
        m_btnStartStop->setText(!m_recording ? i18n("&Record") : i18n("&Stop Recording"));
        m_btnStartStop->setEnabled(true);
    } else {
        m_btnStartStop->setText(i18n("&Record"));
        m_btnStartStop->setEnabled(false);
    }
}


#include "recording-monitor.moc"
