/***************************************************************************
                          recording.cpp  -  description
                             -------------------
    begin                : Mi Aug 27 2003
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

#include "radiostation.h"
#include "errorlog_interfaces.h"
#include "fileringbuffer.h"
#include "utils.h"

#include "recording.h"
#include "recording-configuration.h"
#include "soundstreamevent.h"
#include "recording-monitor.h"
#include "encoder_mp3.h"
#include "encoder_ogg.h"
#include "encoder_pcm.h"

#include <QEvent>
#include <QApplication>

#include <kconfig.h>
#include <kaboutdata.h>


///////////////////////////////////////////////////////////////////////
//// plugin library functions

static KAboutData aboutDataRecording()
{
    KAboutData about("Recording",
                     PROJECT_NAME,
                     ki18nc("@title", "Recording"),
                     KRADIO_VERSION,
                     ki18nc("@title", "Recording Plugin"),
                     KAboutData::License_GPL,
                     ki18nc("@info:credit", "(c) 2002-2005 Martin Witte"),
                     KLocalizedString(),
                     "http://sourceforge.net/projects/kradio",
                     "emw-kradio@nocabal.de");
    about.addAuthor(ki18nc("@info:credit", "Martin Witte"), KLocalizedString(), "emw-kradio@nocabal.de");
    return about;
}

KRADIO_EXPORT_PLUGIN2(
    Recording,           aboutDataRecording(),
    RecordingMonitor,    aboutDataRecordingMonitor()
)

///////////////////////////////////////////////////////////////////////

Recording::Recording(const QString &instanceID, const QString &name)
    : PluginBase(instanceID, name, i18n("KRadio Recording Plugin")),
      m_config()
{
}


Recording::~Recording()
{
    QMap<SoundStreamID, RecordingEncoding*>::iterator it  = m_EncodingThreads.begin();
    QMap<SoundStreamID, RecordingEncoding*>::iterator end = m_EncodingThreads.end();
    for (; it != end; ++it) {
        sendStopRecording(it.key());
    }
}


bool Recording::connectI(Interface *i)
{
    bool a = IRecCfg::connectI(i);
    bool b = PluginBase::connectI(i);
    bool c = ISoundStreamClient::connectI(i);
    bool d = IRadioClient::connectI(i);

    return a || b || c || d;
}


bool Recording::disconnectI(Interface *i)
{
    bool a = IRecCfg::disconnectI(i);
    bool b = PluginBase::disconnectI(i);
    bool c = ISoundStreamClient::disconnectI(i);
    bool d = IRadioClient::disconnectI(i);
    return a || b || c || d;
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
        s->register4_queryEnumerateSourceSoundStreams(this);
        s->register4_notifySoundStreamChanged(this);
        s->register4_notifySoundStreamClosed(this);
    }
}

// PluginBase

void Recording::saveState (KConfigGroup &c) const
{
    PluginBase::saveState(c);

    m_config.saveConfig(c);
}


void Recording::restoreState (const KConfigGroup &c)
{
    PluginBase::restoreState(c);

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
                          "kradio_record"
                         );
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

bool   Recording::setRecordingDirectory(const QString &dir, const recordingTemplate_t &templ)
{
    if (m_config.m_Directory != dir || m_config.m_template != templ) {
        m_config.m_Directory        = dir;
        m_config.m_template         = templ;
        notifyRecordingDirectoryChanged(dir, templ);
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
            for (QMap<SoundStreamID,FileRingBuffer*>::iterator it = m_PreRecordingBuffers.begin(); it != m_PreRecordingBuffers.end(); ++it) {
                if (*it != NULL) {
                    delete *it;
                }
                *it = new FileRingBuffer(m_config.m_Directory + "/kradio-prerecord-"+QString::number(it.key().getID()), m_config.m_PreRecordingSeconds * m_config.m_SoundFormat.m_SampleRate * m_config.m_SoundFormat.frameSize());
                SoundFormat sf = m_config.m_SoundFormat;
                sendStartCaptureWithFormat(it.key(), sf, sf, false);
            }
        }
        else {
            for (QMap<SoundStreamID,FileRingBuffer*>::iterator it = m_PreRecordingBuffers.begin(); it != m_PreRecordingBuffers.end(); ++it) {
                if (*it != NULL) {
                    sendStopCapture(it.key());
                    delete *it;
                    *it = NULL;
                }
            }
            // we should not clear it in order to directly start prerecording once the option is changed while playback
            // is running. therefore we are keeping the NULL pointers (see above)
            //m_PreRecordingBuffers.clear();
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

void Recording::getRecordingDirectory(QString &dir, recordingTemplate_t &templ) const
{
    dir   = m_config.m_Directory;
    templ = m_config.m_template;
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
    setRecordingDirectory(c.m_Directory, c.m_template);
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
    // allways set the NULL pointer in order to get an entry in our map. Thereby we
    // can directly start prerecording on running playbacks once the option is enabled
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
        FileRingBuffer *buf = m_PreRecordingBuffers[id];
        if (buf) {
            delete m_PreRecordingBuffers[id];
        }

        m_PreRecordingBuffers.remove(id);
        // only stop capture if prerecording has been running.
        if (buf) {
            sendStopCapture(id);
        }
    }
    return false;
}

bool Recording::startRecording(SoundStreamID id, const recordingTemplate_t &templ)
{
    SoundFormat realFormat = m_config.m_SoundFormat;
    return sendStartRecordingWithFormat(id, realFormat, realFormat, templ);
}

bool Recording::startRecordingWithFormat(SoundStreamID id, const SoundFormat &sf, SoundFormat &real_format, const recordingTemplate_t &templ)
{
    if (!sendStartCaptureWithFormat(id, sf, real_format, /* force_format = */ true)) {
        logError(i18n("start capture not handled"));
        sendStopCapture(id);
        sendStopRecording(id);
        return false;
    }

    RecordingConfig cfg = m_config;
    cfg.m_SoundFormat   = real_format;
    if (templ.filename.length()) {
        cfg.m_template = templ;
    }

    logInfo(i18n("Recording starting"));
    if (!startEncoder(id, cfg)) {
        logError(i18n("starting encoding thread failed"));
        sendStopCapture(id);
        sendStopRecording(id);
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
        consumed_size = (consumed_size == SIZE_T_DONT_CARE) ? n : qMin(consumed_size, n);
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
                    logError(i18n("could not read sufficient data"));
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
                logWarning(i18n("Encoder input buffer overflow (buffer configuration problem?). Skipped %1 input bytes", QString::number(remSize)));
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
        consumed_size = (consumed_size == SIZE_T_DONT_CARE) ? size - remSize : qMin(consumed_size, size - remSize);

        return true;
    }
    return false;
}




bool Recording::startEncoder(SoundStreamID ssid, const RecordingConfig &_cfg)
{
    if (m_EncodingThreads.contains(ssid))
        return false;
    
    RecordingConfig cfg = _cfg;

    SoundStreamID encID = createNewSoundStream(ssid, false);
    m_RawStreams2EncodedStreams[ssid] = encID;
    m_EncodedStreams2RawStreams[encID] = ssid;

    QString ext = ".wav";
    switch (m_config.m_OutputFormat) {
        case RecordingConfig::outputWAV:  ext = ".wav";  break;
        case RecordingConfig::outputAIFF: ext = ".aiff"; break;
        case RecordingConfig::outputAU:   ext = ".au";   break;
#ifdef HAVE_LAME
        case RecordingConfig::outputMP3:  ext = ".mp3";  break;
#endif
#ifdef HAVE_OGG
        case RecordingConfig::outputOGG:  ext = ".ogg";  break;
#endif
        case RecordingConfig::outputRAW:  ext = ".raw";  break;
        default:                          ext = ".wav";  break;
    }


    const RadioStation *rs          = NULL;
    querySoundStreamRadioStation(ssid, rs);
    int                 stationIdx  = rs ? queryStationIdx(*rs) : -1;

    cfg.m_template.realizeTemplateParameters(rs, stationIdx);


    if (!cfg.m_template.filename.startsWith("/")) {
        cfg.m_template.filename = cfg.m_Directory + "/" + cfg.m_template.filename;
    }

    QString output = cfg.m_template.filename + ext;

    logInfo(i18n("Recording::outputFile: %1", output));

    RecordingEncoding *thread = NULL;
    switch (m_config.m_OutputFormat) {
#ifdef HAVE_LAME
        case RecordingConfig::outputMP3:
            thread = new RecordingEncodingMP3(this, ssid, cfg, rs, output);
            break;
#endif
#ifdef HAVE_OGG
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

        // FIXME: set a timer and do waiting "in background"
        if (!thread->wait(5000)) {
            //m_context.setError();
            logError(i18n("The encoding thread did not finish. It will be killed now."));
            thread->terminate();
            thread->wait();
        } else {
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
                        logError(i18n("Recording::notifySoundStreamData(encoded data): Receivers skipped %1 Bytes", step->size() - consumed_size));
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


bool Recording::enumerateSourceSoundStreams(QMap<QString, SoundStreamID> &list) const
{
    foreach (const SoundStreamID &sid, m_RawStreams2EncodedStreams) {
        QString tmp = QString::null;
        getSoundStreamDescription(sid, tmp);
        list[tmp] = sid;
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
        b  = m_EncodingThreads[id]->isRunning();
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
