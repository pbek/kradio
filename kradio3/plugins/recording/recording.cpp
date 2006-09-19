/***************************************************************************
                          recording.cpp  -  description
                             -------------------
    begin                : Mi Aug 27 2003
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

#include "../../src/radio-stations/radiostation.h"
#include "../../src/interfaces/errorlog-interfaces.h"
#include "../../src/libkradio-gui/aboutwidget.h"
#include "../../src/libkradio/fileringbuffer.h"
#include "../../src/libkradio/utils.h"

#include "recording.h"
#include "recording-configuration.h"
#include "soundstreamevent.h"
#include "recording-monitor.h"
#include "encoder_mp3.h"
#include "encoder_ogg.h"
#include "encoder_pcm.h"

#include <qevent.h>
#include <qapplication.h>
#include <qregexp.h>

#include <kconfig.h>
#include <kdeversion.h>

#include <kaboutdata.h>


///////////////////////////////////////////////////////////////////////
//// plugin library functions

PLUGIN_LIBRARY_FUNCTIONS2(
    Recording, "KRadio Recording Plugin",
    RecordingMonitor, "KRadio Recording Monitor"
);

///////////////////////////////////////////////////////////////////////

Recording::Recording(const QString &name)
    : QObject(NULL, NULL),
      PluginBase(name, i18n("KRadio Recording Plugin")),
      m_config()
{
}


Recording::~Recording()
{
    QMapIterator<SoundStreamID, RecordingEncoding*> it = m_EncodingThreads.begin();
    QMapIterator<SoundStreamID, RecordingEncoding*> end = m_EncodingThreads.end();
    for (; it != end; ++it) {
        sendStopRecording(it.key());
    }
}


bool Recording::connectI(Interface *i)
{
    bool a = IRecCfg::connectI(i);
    bool b = PluginBase::connectI(i);
    bool c = ISoundStreamClient::connectI(i);
    return a || b || c;
}


bool Recording::disconnectI(Interface *i)
{
    bool a = IRecCfg::disconnectI(i);
    bool b = PluginBase::disconnectI(i);
    bool c = ISoundStreamClient::disconnectI(i);
    return a || b || c;
}


void Recording::noticeConnectedI (ISoundStreamServer *s, bool pointer_valid)
{
    ISoundStreamClient::noticeConnectedI(s, pointer_valid);
    if (s && pointer_valid) {
        s->register4_sendStartPlayback(this);
        s->register4_sendStopPlayback(this);
        s->register4_sendStartRecording(this);
        s->register4_sendStartRecordingWithFormat(this);
        s->register4_notifySoundStreamData(this);
        s->register4_sendStopRecording(this);
        s->register4_queryIsRecordingRunning(this);
        s->register4_querySoundStreamDescription(this);
        s->register4_querySoundStreamRadioStation(this);
        s->register4_queryEnumerateSoundStreams(this);
        s->register4_notifySoundStreamChanged(this);
        s->register4_notifySoundStreamClosed(this);
    }
}

// PluginBase

void Recording::saveState (KConfig *c) const
{
    c->setGroup(QString("recording-") + PluginBase::name());
    m_config.saveConfig(c);
}


void Recording::restoreState (KConfig *c)
{
    c->setGroup(QString("recording-") + PluginBase::name());
    RecordingConfig cfg;
    cfg.restoreConfig(c);
    setRecordingConfig(cfg);
    //notifyRecordingConfigChanged(m_config);
}


ConfigPageInfo  Recording::createConfigurationPage()
{
    RecordingConfiguration *c = new RecordingConfiguration(NULL);
    connectI(c);
    return ConfigPageInfo(c,
                          i18n("Recording"),
                          i18n("Recording"),
                          "kradio_record");
}


AboutPageInfo Recording::createAboutPage()
{
/*    KAboutData aboutData("kradio",
                         NULL,
                         NULL,
                         I18N_NOOP("Recording Monitor for KRadio"),
                         KAboutData::License_GPL,
                         "(c) 2002-2005 Martin Witte",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  "", "witte@kawo1.rwth-aachen.de");

    return AboutPageInfo(
              new KRadioAboutWidget(aboutData, KRadioAboutWidget::AbtTabbed),
              i18n("Recording"),
              i18n("Recording Plugin"),
              "kradio_record"
           );*/
    return AboutPageInfo();
}


// IRecCfg

bool   Recording::setEncoderBuffer     (size_t BufferSize, size_t BufferCount)
{
    if (m_config.m_EncodeBufferSize  != BufferSize ||
        m_config.m_EncodeBufferCount != BufferCount)
    {
        m_config.m_EncodeBufferSize  = BufferSize;
        m_config.m_EncodeBufferCount = BufferCount;
        notifyEncoderBufferChanged(BufferSize, BufferCount);
    }
    return true;
}

bool   Recording::setSoundFormat       (const SoundFormat &sf)
{
    if (m_config.m_SoundFormat != sf) {
        m_config.m_SoundFormat = sf;
        notifySoundFormatChanged(sf);
    }
    return true;
}

bool   Recording::setMP3Quality        (int q)
{
    if (m_config.m_mp3Quality != q) {
        m_config.m_mp3Quality = q;
        notifyMP3QualityChanged(q);
    }
    return true;
}

bool   Recording::setOggQuality        (float q)
{
    if (m_config.m_oggQuality != q) {
        m_config.m_oggQuality = q;
        notifyOggQualityChanged(q);
    }
    return true;
}

bool   Recording::setRecordingDirectory(const QString &dir)
{
    if (m_config.m_Directory != dir) {
        m_config.m_Directory = dir;
        notifyRecordingDirectoryChanged(dir);
    }
    return true;
}

bool   Recording::setOutputFormat (RecordingConfig::OutputFormat of)
{
    if (m_config.m_OutputFormat != of) {
        m_config.m_OutputFormat = of;
        notifyOutputFormatChanged(of);
    }
    return true;
}

bool   Recording::setPreRecording (bool enable, int seconds)
{
    if (m_config.m_PreRecordingEnable != enable || m_config.m_PreRecordingSeconds != seconds) {
        m_config.m_PreRecordingEnable = enable;
        m_config.m_PreRecordingSeconds = seconds;

        if (enable) {
            for (QMapIterator<SoundStreamID,FileRingBuffer*> it = m_PreRecordingBuffers.begin(); it != m_PreRecordingBuffers.end(); ++it) {
                if (*it != NULL) {
                    delete *it;
                }
                *it = new FileRingBuffer(m_config.m_Directory + "/kradio-prerecord-"+QString::number(it.key().getID()), m_config.m_PreRecordingSeconds * m_config.m_SoundFormat.m_SampleRate * m_config.m_SoundFormat.frameSize());
                SoundFormat sf = m_config.m_SoundFormat;
                sendStartCaptureWithFormat(it.key(), sf, sf, false);
            }
        }
        else {
            for (QMapIterator<SoundStreamID,FileRingBuffer*> it = m_PreRecordingBuffers.begin(); it != m_PreRecordingBuffers.end(); ++it) {
                if (*it != NULL) {
                    sendStopCapture(it.key());
                    delete *it;
                }
            }
            m_PreRecordingBuffers.clear();
        }

        notifyPreRecordingChanged(enable, seconds);
    }
    return true;
}

void   Recording::getEncoderBuffer(size_t &BufferSize, size_t &BufferCount) const
{
    BufferSize  = m_config.m_EncodeBufferSize;
    BufferCount = m_config.m_EncodeBufferCount;
}

const SoundFormat &Recording::getSoundFormat () const
{
    return m_config.m_SoundFormat;
}

int Recording::getMP3Quality () const
{
    return m_config.m_mp3Quality;
}

float Recording::getOggQuality () const
{
    return m_config.m_oggQuality;
}

const QString &Recording::getRecordingDirectory() const
{
    return m_config.m_Directory;
}

RecordingConfig::OutputFormat Recording::getOutputFormat() const
{
    return m_config.m_OutputFormat;
}

bool Recording::getPreRecording(int &seconds) const
{
    seconds = m_config.m_PreRecordingSeconds;
    return m_config.m_PreRecordingEnable;
}

const RecordingConfig &Recording::getRecordingConfig() const
{
    return m_config;
}

bool Recording::setRecordingConfig(const RecordingConfig &c)
{
    setEncoderBuffer     (c.m_EncodeBufferSize, c.m_EncodeBufferCount);
    setSoundFormat       (c.m_SoundFormat);
    setMP3Quality        (c.m_mp3Quality);
    setOggQuality        (c.m_oggQuality);
    setRecordingDirectory(c.m_Directory);
    setOutputFormat      (c.m_OutputFormat);
    setPreRecording      (c.m_PreRecordingEnable, c.m_PreRecordingSeconds);

    m_config = c;

    notifyRecordingConfigChanged(m_config);

    return true;
}


// ISoundStreamClient
bool Recording::startPlayback(SoundStreamID id)
{
    if (m_PreRecordingBuffers.contains(id))
        delete m_PreRecordingBuffers[id];
    m_PreRecordingBuffers[id] = NULL;
    if (m_config.m_PreRecordingEnable) {
        m_PreRecordingBuffers[id] = new FileRingBuffer(m_config.m_Directory + "/kradio-prerecord-"+QString::number(id.getID()), m_config.m_PreRecordingSeconds * m_config.m_SoundFormat.m_SampleRate * m_config.m_SoundFormat.frameSize());
        SoundFormat sf = m_config.m_SoundFormat;
        sendStartCaptureWithFormat(id, sf, sf, false);
    }
    return false;
}

bool Recording::stopPlayback(SoundStreamID id)
{
    if (m_PreRecordingBuffers.contains(id)) {
        if (m_PreRecordingBuffers[id])
            delete m_PreRecordingBuffers[id];
        m_PreRecordingBuffers.remove(id);
        sendStopCapture(id);
    }
    return false;
}

bool Recording::startRecording(SoundStreamID id)
{

/*    FileRingBuffer *test = new FileRingBuffer("/tmp/ringbuffertest", 2048);
    char buffer1[1024];
    char buffer2[1024];
    char buffer3[1024];
    for (int i = 0; i < 1024; ++i) {
        buffer1[i] = 'a';
        buffer2[i] = 'b';
        buffer3[i] = 'c';
    }
    test->addData(buffer1, 1024);
    test->addData(buffer2, 1024);
    test->removeData(1024);
    test->addData(buffer3, 1024);
*/

    SoundFormat realFormat = m_config.m_SoundFormat;
    return sendStartRecordingWithFormat(id, realFormat, realFormat);
}

bool Recording::startRecordingWithFormat(SoundStreamID id, const SoundFormat &sf, SoundFormat &real_format)
{
    if (!sendStartCaptureWithFormat(id, sf, real_format, /* force_format = */ true)) {
        logError(i18n("start capture not handled"));
        return false;
    }

    RecordingConfig cfg = m_config;
    cfg.m_SoundFormat   = real_format;

    logInfo(i18n("Recording starting"));
    if (!startEncoder(id, cfg)) {
        logError(i18n("starting encoding thread failed"));
        sendStopCapture(id);
        return false;
    }

    return true;
}


bool Recording::stopRecording(SoundStreamID id)
{
    if (m_EncodingThreads.contains(id)) {
        sendStopCapture(id);
        if (m_config.m_PreRecordingEnable) {
            if (!m_PreRecordingBuffers.contains(id)) {
                if (m_PreRecordingBuffers[id] != NULL) {
                    delete m_PreRecordingBuffers[id];
                }
                bool b = false;
                queryIsPlaybackRunning(id, b);
                if (b) {
                    m_PreRecordingBuffers[id] = new FileRingBuffer(m_config.m_Directory + "/kradio-prerecord-"+QString::number(id.getID()), m_config.m_PreRecordingSeconds * m_config.m_SoundFormat.m_SampleRate * m_config.m_SoundFormat.frameSize());
                } else {
                    m_PreRecordingBuffers[id] = NULL;
                }
            }
        }
        stopEncoder(id);
        return true;
    }
    return false;
}



bool Recording::noticeSoundStreamData(SoundStreamID id,
    const SoundFormat &/*sf*/, const char *data, size_t size, size_t &consumed_size,
    const SoundMetaData &md
)
{
    if (m_PreRecordingBuffers.contains(id) && m_PreRecordingBuffers[id] != NULL) {

        FileRingBuffer &fbuf = *m_PreRecordingBuffers[id];
        if (fbuf.getFreeSize() < size) {
            fbuf.removeData(size - fbuf.getFreeSize());
        }
        size_t n = fbuf.addData(data, size);
        consumed_size = (consumed_size == SIZE_T_DONT_CARE) ? n : min(consumed_size, n);
//         if (n != size) {
//             logDebug("recording packet: was not written completely to tmp buf");
//         }

//         //BEGIN DEBUG
//         char tmp[4096];
//         for (unsigned int i = 0; i < sizeof(tmp); ++i) { tmp[i] = 0; }
//         if (fbuf.getFreeSize() < sizeof(tmp)) {
//             fbuf.removeData(sizeof(tmp) - fbuf.getFreeSize());
//         }
//         fbuf.addData((char*)tmp, sizeof(tmp));
//         //END DEBUG

        if (m_EncodingThreads.contains(id)) {

            //logDebug("recording packet: " + QString::number(size));

            RecordingEncoding *thread = m_EncodingThreads[id];

            //logDebug("noticeSoundStreamData thread = " + QString::number((long long)thread, 16));

            size_t        remSize = fbuf.getFillSize();

            while (remSize > 0) {
                size_t  bufferSize = remSize;
                char *buf = thread->lockInputBuffer(bufferSize);
                if (!buf) {
                    // Encoder buffer is full and bigger than remaining data
                    break;
                }
                if (bufferSize > remSize) {
                    bufferSize = remSize;
                }
                if (fbuf.takeData(buf, bufferSize) != bufferSize) {
                    logError(i18n("could not read suffient data"));
                }

                thread->unlockInputBuffer(bufferSize, md);
                remSize -= bufferSize;
            }

            if (remSize == 0) {
                delete m_PreRecordingBuffers[id];
                m_PreRecordingBuffers.remove(id);
            }
        }

        return true;
    }

    else if (m_EncodingThreads.contains(id)) {

        //logDebug("recording packet: " + QString::number(size));

        RecordingEncoding *thread = m_EncodingThreads[id];

        //logDebug("noticeSoundStreamData thread = " + QString::number((long long)thread, 16));

        size_t        remSize = size;
        const char   *remData = data;

        while (remSize > 0) {
            size_t  bufferSize = remSize;
            char *buf = thread->lockInputBuffer(bufferSize);
            if (!buf) {
                logWarning(i18n("Encoder input buffer overflow (buffer configuration problem?). Skipped %1 input bytes").arg(QString::number(remSize)));
                break;
            }
            if (bufferSize > remSize) {
                bufferSize = remSize;
            }
            memcpy(buf, remData, bufferSize);

            thread->unlockInputBuffer(bufferSize, md);
            remSize -= bufferSize;
            remData += bufferSize;
        }
        consumed_size = (consumed_size == SIZE_T_DONT_CARE) ? size - remSize : min(consumed_size, size - remSize);

        return true;
    }
    return false;
}




bool Recording::startEncoder(SoundStreamID ssid, const RecordingConfig &cfg)
{
    if (m_EncodingThreads.contains(ssid))
        return false;

    SoundStreamID encID = createNewSoundStream(ssid, false);
    m_RawStreams2EncodedStreams[ssid] = encID;
    m_EncodedStreams2RawStreams[encID] = ssid;

    QString ext = ".wav";
    switch (m_config.m_OutputFormat) {
        case RecordingConfig::outputWAV:  ext = ".wav";  break;
        case RecordingConfig::outputAIFF: ext = ".aiff"; break;
        case RecordingConfig::outputAU:   ext = ".au";   break;
#ifdef HAVE_LAME_LAME_H
        case RecordingConfig::outputMP3:  ext = ".mp3";  break;
#endif
#ifdef HAVE_LAME_LAME_H
        case RecordingConfig::outputOGG:  ext = ".ogg";  break;
#endif
        case RecordingConfig::outputRAW:  ext = ".raw";  break;
        default:                          ext = ".wav";  break;
    }
    const RadioStation *rs = NULL;
    querySoundStreamRadioStation(ssid, rs);
    QString station = rs  ? rs->name() + "-" : "";
    station.replace(QRegExp("[/*?]"), "_");

    QDate date = QDate::currentDate();
    QTime time = QTime::currentTime();
    QString sdate;

    sdate.sprintf("%d.%d.%d.%d.%d",date.year(),date.month(),date.day(),time.hour(),time.minute());

    QString output = m_config.m_Directory
        + "/kradio-recording-"
        + station
        + sdate
        + ext;

    logInfo(i18n("Recording::outputFile: ") + output);

    RecordingEncoding *thread = NULL;
    switch (m_config.m_OutputFormat) {
#ifdef HAVE_LAME_LAME_H
        case RecordingConfig::outputMP3:
            thread = new RecordingEncodingMP3(this, ssid, cfg, rs, output);
            break;
#endif
#if defined(HAVE_VORBIS_VORBISENC_H) && defined(HAVE_OGG_OGG_H)
        case RecordingConfig::outputOGG:
            thread = new RecordingEncodingOgg(this, ssid, cfg, rs, output);
            break;
#endif
        default:
            thread = new RecordingEncodingPCM(this, ssid, cfg, rs, output);
    }

    //m_encodingThread->openOutput(output, rs);

    if (thread->error()) {
        //m_context.setError();
        logError(thread->errorString());
    } else {
        thread->start();
    }
    // store thread even if it has indicated an error
    m_EncodingThreads[ssid] = thread;

    //logDebug("startEncoder thread = " + QString::number((long long)thread, 16));

    notifySoundStreamCreated(encID);
    return !thread->error();
}


void Recording::stopEncoder(SoundStreamID id)
{
    if (m_EncodingThreads.contains(id)) {

        RecordingEncoding *thread = m_EncodingThreads[id];

        thread->setDone();

        //logDebug("stopEncoder thread = " + QString::number((long long)thread, 16));
        //logDebug("stopEncoder thread error = " + QString::number(thread->error(), 16));

#if (KDE_VERSION_MAJOR >= 3) && (KDE_VERSION_MINOR >= 1)
        // FIXME: set a timer and do waiting "in background"
        if (!thread->wait(5000)) {
            //m_context.setError();
            logError(i18n("The encoding thread did not finish. It will be killed now."));
            thread->terminate();
            thread->wait();
        } else {
#else
            logError(i18n("Waiting for encoding thread to terminate."));
            thread->wait();
#endif
            if (thread->error()) {
                //m_context.setError();
                logError(thread->errorString());
            } else {
                //Q_UINT64 size = thread->encodedSize();
                //m_context.setEncodedSize(low, high);
                //notifyRecordingContextChanged(m_context);
            }
        }
        delete thread;
        m_EncodingThreads.remove(id);
        SoundStreamID encID = m_RawStreams2EncodedStreams[id];
        m_EncodedStreams2RawStreams.remove(encID);
        m_RawStreams2EncodedStreams.remove(id);
        sendStopPlayback(encID);
        closeSoundStream(encID);
        logInfo(i18n("Recording stopped"));
    }
}


bool Recording::event(QEvent *_e)
{
    if (SoundStreamEvent::isSoundStreamEvent(_e)) {
        SoundStreamEvent *e = static_cast<SoundStreamEvent*>(_e);
        SoundStreamID id = e->getSoundStreamID();

        if (m_EncodingThreads.contains(id)) {

            RecordingEncoding *thread = m_EncodingThreads[id];

            //logDebug("Recording::event: thread = " + QString::number((long long)thread, 16));

            if (thread->error()) {
                logError(thread->errorString());
                //m_context.setError();
                stopEncoder(id);
            } else {
                //Q_UINT64 size = thread->encodedSize();
                //m_context.setEncodedSize(low, high);
                //notifyRecordingContextChanged(m_context);
                if (e->type() == EncodingTerminated) {
                    stopEncoder(id);
                } else if (e->type() == EncodingStep) {
                    SoundStreamEncodingStepEvent *step = static_cast<SoundStreamEncodingStepEvent*>(e);
                    size_t consumed_size = SIZE_T_DONT_CARE;
                    notifySoundStreamData(m_RawStreams2EncodedStreams[id], thread->config().m_SoundFormat,
                                          step->data(), step->size(), consumed_size, step->metaData());
                    if (consumed_size != SIZE_T_DONT_CARE && consumed_size < step->size()) {
                        logError(i18n("Recording::notifySoundStreamData(encoded data): Receivers skipped %1 Bytes").arg(step->size() - consumed_size));
                    }
                }
            }
        }
        return true;
    } else {
        return QObject::event(_e);
    }
}


bool Recording::getSoundStreamDescription(SoundStreamID id, QString &descr) const
{
    if (m_EncodedStreams2RawStreams.contains(id)) {
        if (querySoundStreamDescription(m_EncodedStreams2RawStreams[id], descr)) {
            descr = name() + " - " + descr;
            return true;
        }
    }
    return false;
}


bool Recording::getSoundStreamRadioStation(SoundStreamID id, const RadioStation *&rs) const
{
    if (m_EncodedStreams2RawStreams.contains(id)) {
        if (querySoundStreamRadioStation(m_EncodedStreams2RawStreams[id], rs)) {
            return true;
        }
    }
    return false;
}


bool Recording::enumerateSoundStreams(QMap<QString, SoundStreamID> &list) const
{
    QMapConstIterator<SoundStreamID,SoundStreamID> end = m_RawStreams2EncodedStreams.end();
    for (QMapConstIterator<SoundStreamID,SoundStreamID> it = m_RawStreams2EncodedStreams.begin(); it != end; ++it) {
        QString tmp = QString::null;
        getSoundStreamDescription(*it, tmp);
        list[tmp] = *it;
    }
    return m_RawStreams2EncodedStreams.count() > 0;
}


bool Recording::noticeSoundStreamChanged(SoundStreamID id)
{
    if (m_RawStreams2EncodedStreams.contains(id)) {
        notifySoundStreamChanged(m_RawStreams2EncodedStreams[id]);
        return true;
    }
    return false;
}


bool Recording::isRecordingRunning(SoundStreamID id, bool &b, SoundFormat &sf) const
{
    if (m_EncodingThreads.contains(id)) {
        b  = m_EncodingThreads[id]->running();
        sf = getSoundFormat();
        return true;
    }
    return false;
}


bool Recording::noticeSoundStreamClosed(SoundStreamID id)
{
    if (m_PreRecordingBuffers.contains(id)) {
        if (m_PreRecordingBuffers[id])
            delete m_PreRecordingBuffers[id];
        m_PreRecordingBuffers.remove(id);
    }

    if (m_EncodingThreads.contains(id)) {
        sendStopRecording(id);
        return true;
    }
    return false;
}


#include "recording.moc"
