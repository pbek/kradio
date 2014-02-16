/***************************************************************************
                          recording-monitor.cpp  -  description
                             -------------------
    begin                : Mo Sep 1 2003
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

#include "recording-monitor.h"
#include "recording-datamonitor.h"
//#include "aboutwidget.h"

#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QLayout>
#include <QtGui/QCheckBox>

#include <kcombobox.h>
#include <klocale.h>
#include <kconfiggroup.h>
#include <kaboutdata.h>

RecordingMonitor::RecordingMonitor(const QString &instanceID, const QString &name)
  : QWidget(NULL),
    WidgetPluginBase(this, instanceID, name, i18n("Recording Monitor")),
    m_recording(false),
    m_defaultStreamDescription(QString())
{
    setWindowTitle(i18n("KRadio Recording Monitor"));

    QVBoxLayout *l = new QVBoxLayout(this); //, 10, 4);
    QGridLayout *l0 = new QGridLayout(); //, 6, 2);
    l->addLayout(l0);

    l0->addWidget(m_labelHdrSoundStream      = new QLabel(i18n("Sound Stream"),    this), 0, 0);
    l0->addWidget(m_comboSoundStreamSelector = new KComboBox(                     this), 0, 1);
/*    l0->addWidget(m_labelHdrStatus           = new QLabel(i18n("Status"),         this), 1, 0);
    l0->addWidget(m_labelStatus              = new QLabel(i18n("<undefined>"),    this), 1, 1);*/
    l0->addWidget(m_labelHdrFilename         = new QLabel(i18n("Recording File"), this), 2, 0);
    l0->addWidget(m_labelFileName            = new QLabel(i18n("<undefined>"),    this), 2, 1);
    l0->addWidget(m_labelHdrSize             = new QLabel(i18n("File Size"),      this), 3, 0);
    l0->addWidget(m_labelSize                = new QLabel(i18n("<undefined>"),    this), 3, 1);
    l0->addWidget(m_labelHdrTime             = new QLabel(i18n("Recording Time"), this), 4, 0);
    l0->addWidget(m_labelTime                = new QLabel(i18n("<undefined>"),    this), 4, 1);
    l0->addWidget(m_labelHdrRate             = new QLabel(i18n("Sample Rate"),    this), 5, 0);
    l0->addWidget(m_labelRate                = new QLabel(i18n("<undefined>"),    this), 5, 1);

    m_labelHdrSoundStream->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//     m_labelHdrStatus     ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_labelHdrFilename   ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_labelHdrSize       ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_labelHdrRate       ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_labelHdrTime       ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QPushButton *close     = new QPushButton(i18n("&Close"), this);
    m_btnStartStop         = new QPushButton(i18n("&Record"), this);
    QObject::connect(close, SIGNAL(clicked()), this, SLOT(hide()));
    QObject::connect(m_btnStartStop, SIGNAL(clicked()), this, SLOT(slotStartStopRecording()));

    m_dataMonitor = new RecordingDataMonitor(this);
    m_dataMonitor->setEnabled(false);

    QHBoxLayout *hl0 = new QHBoxLayout();
    l->addLayout(hl0);
    hl0->addWidget(m_dataMonitor);

    QHBoxLayout *hl2 = new QHBoxLayout();
    l->addLayout(hl2);
    hl2->addItem(new QSpacerItem(10, 1));
    hl2->addWidget(close);
    hl2->addWidget(m_btnStartStop);
    hl2->addItem(new QSpacerItem(10, 1));


    m_comboSoundStreamSelector->addItem(i18n("nothing"));
    QObject::connect(m_comboSoundStreamSelector, SIGNAL(activated(int)), this, SLOT(slotStreamSelected(int)));

    updateRecordingButton();
    slotStreamSelected(0);
}


RecordingMonitor::~RecordingMonitor()
{
}

// WidgetPluginBase

void   RecordingMonitor::saveState (KConfigGroup &config) const
{
    WidgetPluginBase::saveState(config);
}


void   RecordingMonitor::restoreState (const KConfigGroup &config)
{
    WidgetPluginBase::restoreState(config, false);
}


bool   RecordingMonitor::connectI(Interface *i)
{
    bool a = ISoundStreamClient::connectI(i);
    bool b = IRadioClient      ::connectI(i);
    bool c = WidgetPluginBase  ::connectI(i);
    return a || b || c;
}

bool   RecordingMonitor::disconnectI(Interface *i)
{
    bool a = ISoundStreamClient::disconnectI(i);
    bool b = IRadioClient      ::disconnectI(i);
    bool c = WidgetPluginBase  ::disconnectI(i);
    if (a) {
        m_comboSoundStreamSelector->clear();
        m_SoundStreamID2idx.clear();
        m_idx2SoundStreamID.clear();
        m_comboSoundStreamSelector->addItem(i18n("nothing"));
    }
    return a || b || c;
}

// ISoundStreamClient

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
        queryEnumerateSourceSoundStreams(tmp);

        m_comboSoundStreamSelector->clear();
        m_SoundStreamID2idx.clear();
        m_idx2SoundStreamID.clear();
        m_comboSoundStreamSelector->addItem(i18n("nothing"));
        QMap<QString, SoundStreamID>::const_iterator end = tmp.end();
        for (QMap<QString, SoundStreamID>::const_iterator it = tmp.begin(); it != end; ++it) {
            int idx = m_comboSoundStreamSelector->count();
            m_comboSoundStreamSelector->addItem(it.key());
            m_idx2SoundStreamID[idx] = *it;
            m_SoundStreamID2idx[*it] = idx;
        }
    }
}

ConfigPageInfo  RecordingMonitor::createConfigurationPage()
{
    return ConfigPageInfo();
}

/*AboutPageInfo   RecordingMonitor::createAboutPage()
{*/
/*    KAboutData aboutData("kradio4",
                         NULL,
                         NULL,
                         I18N_NOOP("Recording Monitor Plugin for KRadio"),
                         KAboutData::License_GPL,
                         "(c) 2002-2005 Martin Witte",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  "", "emw-kradio@nocabal.de");

    return AboutPageInfo(
              new KRadioAboutWidget(aboutData, KRadioAboutWidget::AbtTabbed),
              i18n("Recording Monitor"),
              i18n("Recording Monitor Plugin"),
              "goto"
           );
*/
//     return AboutPageInfo();
// }

void RecordingMonitor::setVisible(bool v)
{
    pSetVisible(v);
    QWidget::setVisible(v);
}



void RecordingMonitor::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    WidgetPluginBase::pShowEvent(e);
    //m_comboSoundStreamSelector->setCurrentItem(1);
    //slotStreamSelected(1);
}


void RecordingMonitor::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);
    WidgetPluginBase::pHideEvent(e);
    m_comboSoundStreamSelector->setCurrentIndex(0);
    slotStreamSelected(0);
}


void RecordingMonitor::slotStartStopRecording()
{
    if (m_currentStream.isValid()) {
        if (m_recording) {
            sendStopRecording(m_currentStream);
        } else {
            if (!queryIsPowerOn())
                sendPowerOn();
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
    m_comboSoundStreamSelector->addItem(tmp);
    m_idx2SoundStreamID[idx] = id;
    m_SoundStreamID2idx[id]  = idx;

    if (tmp == m_defaultStreamDescription) {
        m_comboSoundStreamSelector->setCurrentIndex(idx);
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
        QMap<SoundStreamID, int>::iterator end = m_SoundStreamID2idx.end();
        for (QMap<SoundStreamID, int>::iterator it = m_SoundStreamID2idx.begin(); it != end; ++it) {
            if (*it > idx) {
                (*it)--;
            }
            m_idx2SoundStreamID[*it] = it.key();
        }
        m_comboSoundStreamSelector->removeItem(idx);
        slotStreamSelected(m_comboSoundStreamSelector->currentIndex());
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
        m_comboSoundStreamSelector->setItemText(idx, tmp);
        if (idx == m_comboSoundStreamSelector->currentIndex()) {
            m_defaultStreamDescription = tmp;
        }
        return true;
    }
    return false;
}

bool RecordingMonitor::startRecordingWithFormat(SoundStreamID id, const SoundFormat &/*sf*/, SoundFormat &/*real_format*/, const recordingTemplate_t &/*filenameTemplate*/)
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
    const SoundFormat &sf, const char *data, size_t size, size_t &consumed_size,
    const SoundMetaData &md
)
{
    SoundStreamID x = id;
    int cidx = m_comboSoundStreamSelector->currentIndex();
    SoundStreamID y = m_idx2SoundStreamID[cidx];
    if (y == x) {

        m_labelFileName->setText(md.url().pathOrUrl());

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
            m_dataMonitor->noticeSoundStreamData(id, sf, data, size, consumed_size, md);
        } else {
            m_dataMonitor->setEnabled(false);
        }

        QString str_size;
        str_size = KGlobal::locale()->formatByteSize(B);
        m_labelSize->setText(str_size);

        m_labelRate->setText(i18n("%1 Hz", sf.m_SampleRate));
        return true;
    }
    return false;
}

// IRadioClient
bool RecordingMonitor::noticePowerChanged(bool on)
{
    if (!on) {
        m_dataMonitor->reset();
    }
    slotStreamSelected(m_comboSoundStreamSelector->currentIndex());
    return true;
}


// Qt Slots

void RecordingMonitor::slotStreamSelected(int idx)
{
    SoundStreamID old_id = m_currentStream;
    if (old_id.isValid()) {
        sendStopCapture(old_id);
    }

    SoundStreamID id = m_idx2SoundStreamID.contains(idx) ? m_idx2SoundStreamID[idx] : SoundStreamID::InvalidID;
    if (id.isValid()) {
        m_defaultStreamDescription = m_comboSoundStreamSelector->itemText(idx);
    }

    bool enable = id.isValid() && queryIsPowerOn();
    if (enable) {
        SoundFormat sf;
        sendStartCaptureWithFormat(id, sf, sf);
        if (old_id != id) {
            m_labelSize     ->setText(i18n("0.000 kB"));
            m_labelTime     ->setText(i18n("00:00:00.00"));
            m_labelRate     ->setText(i18n("<undefined>"));
            m_labelFileName ->setText(i18n("<undefined>"));
//             m_labelStatus   ->setText(i18n("<undefined>"));
        }
    } else {
        if (old_id != id) {
            m_labelSize     ->setText(i18n("<undefined>"));
            m_labelTime     ->setText(i18n("<undefined>"));
            m_labelRate     ->setText(i18n("<undefined>"));
            m_labelFileName ->setText(i18n("<undefined>"));
//             m_labelStatus   ->setText(i18n("<undefined>"));
        }
    }

    m_dataMonitor   ->setEnabled(enable);
    m_labelSize     ->setEnabled(enable);
    m_labelTime     ->setEnabled(enable);
    m_labelRate     ->setEnabled(enable);
    m_labelFileName ->setEnabled(enable);
//     m_labelStatus   ->setEnabled(enable);

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
