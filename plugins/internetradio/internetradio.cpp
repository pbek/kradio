/***************************************************************************
                          internetradio.cpp  -  description
                             -------------------
    begin                : Mon Feb 23 CET 2009
    copyright            : (C) 2009 by Ernst Martin Witte
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

#include <QList>

#include <kconfiggroup.h>
#include <KAboutData>
#include <klocalizedstring.h>

#include <unistd.h>

#include "stationlist.h"
#include "internetradio.h"
#include "decoder_thread.h"

#include "internetradio-configuration.h"
#include "icy_http_handler.h"
#include "mmsx_handler.h"


#define MAX_BUFFER_CHUNKS           16
#define MIN_BUFFER_CHUNKS4PLAYBACK  (MAX_BUFFER_CHUNKS/3)

///////////////////////////////////////////////////////////////////////

static KAboutData aboutData()
{
    KAboutData about("InternetRadio",
                     i18nc("@title", "Internet Radio"),
                     KRADIO_VERSION,
                     i18nc("@title", "Pseudo radio device for Internet radio stream support"),
                     KAboutLicense::LicenseKey::GPL,
                     NULL,
                     NULL,
                     "http://sourceforge.net/projects/kradio",
                     "emw-kradio@nocabal.de");
    return about;
}

KRADIO_EXPORT_PLUGIN(InternetRadio, aboutData())
#include "internetradio.moc"


///////////////////////////////////////////////////////////////////////



InternetRadio::InternetRadio(const QString &instanceID, const QString &name)
  : PluginBase(instanceID, name, i18n("Internet Radio Plugin")),
    m_powerOn(false),
    m_decoderThread(NULL),
    m_currentStation(InternetRadioStation()),
#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    m_streamReader             (NULL),
#endif
    m_stereoFlag(false),
    m_muted(false),
    m_defaultPlaybackVolume(0.5),
    m_PlaybackMixerID(QString()),
    m_PlaybackMixerChannel(QString()),
    m_PlaybackMixerMuteOnPowerOff(false),
    m_restorePowerOn(false),
    m_RDS_visible(false),
    m_RDS_StationName(QString()),
    m_RDS_RadioText(QString()),
    m_maxStreamProbeSize(8192),
    m_maxStreamAnalyzeTime(0.8),
    m_inputBufferSize(128*1024),
    m_outputBufferSize(512*1024),
    m_watchdogTimeout(4),
    m_watchdogHandlerInService(false),
    m_waitForBufferMinFill(true),
    m_i18nLogPrefix(i18n("Internet Radio Plugin: "))
{
    m_SoundStreamSinkID   = createNewSoundStream(false);
    m_SoundStreamSourceID = m_SoundStreamSinkID;

    QObject::connect(&m_playlistHandler, &PlaylistHandler::sigEOL,            this, &InternetRadio::slotPlaylistEOL);
    QObject::connect(&m_playlistHandler, &PlaylistHandler::sigError,          this, &InternetRadio::slotPlaylistError);
    QObject::connect(&m_playlistHandler, &PlaylistHandler::sigPlaylistLoaded, this, &InternetRadio::slotPlaylistLoaded);
    QObject::connect(&m_playlistHandler, &PlaylistHandler::sigStreamSelected, this, &InternetRadio::slotPlaylistStreamSelected);
    QObject::connect(&m_watchdogTimer,   &QTimer         ::timeout,           this, &InternetRadio::slotWatchdogTimeout);
} // InternetRadio CTOR


InternetRadio::~InternetRadio()
{
    setPower(false);
#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    delete m_streamReader;
    m_streamReader    = NULL;
#endif
}


bool InternetRadio::connectI (Interface *i)
{
    bool a = IRadioDevice      ::connectI(i);
    bool b = IRadioClient      ::connectI(i);
    bool c = IInternetRadio    ::connectI(i);
    bool d = PluginBase        ::connectI(i);
    bool e = ISoundStreamClient::connectI(i);
    return a || b || c || d || e;
}


bool InternetRadio::disconnectI (Interface *i)
{
    bool a = IRadioDevice      ::disconnectI(i);
    bool b = IRadioClient      ::disconnectI(i);
    bool c = IInternetRadio    ::disconnectI(i);
    bool d = PluginBase        ::disconnectI(i);
    bool e = ISoundStreamClient::disconnectI(i);
    return a || b || c || d || e;
}


void InternetRadio::noticeConnectedI (ISoundStreamServer *s, bool pointer_valid)
{
    ISoundStreamClient::noticeConnectedI(s, pointer_valid);
    if (s && pointer_valid) {

        s->register4_notifyPlaybackChannelsChanged(this);
        s->register4_sendStartCaptureWithFormat(this);

        s->register4_queryPlaybackVolume(this);

        s->register4_sendMuteSource(this);
        s->register4_sendUnmuteSource(this);

        s->register4_querySignalQuality(this);
        s->register4_queryHasGoodQuality(this);
        s->register4_queryIsStereo(this);
        s->register4_queryIsSourceMuted(this);

        s->register4_sendPlaybackVolume(this);

        s->register4_querySoundStreamDescription(this);
        s->register4_querySoundStreamRadioStation(this);
        s->register4_queryEnumerateSourceSoundStreams(this);

        s->register4_notifyReadyForPlaybackData(this);
        s->register4_notifySoundStreamClosed(this);
        s->register4_notifySoundStreamSinkRedirected(this);
        s->register4_notifySoundStreamSourceRedirected(this);

        notifySoundStreamCreated(m_SoundStreamSinkID);
        if (m_SoundStreamSinkID != m_SoundStreamSourceID) {
            notifySoundStreamCreated(m_SoundStreamSourceID);
        }
    }
}

void InternetRadio::noticeConnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid)
{
    if (i && pointer_valid && i->getSoundStreamClientID() == m_PlaybackMixerID) {
        setPlaybackMixer(m_PlaybackMixerID, m_PlaybackMixerChannel, m_PlaybackMixerMuteOnPowerOff, /* force = */ true);
    }
}


bool InternetRadio::noticePlaybackChannelsChanged(const QString & client_id, const QStringList &/*channels*/)
{
    if (client_id == m_PlaybackMixerID) {
        setPlaybackMixer(m_PlaybackMixerID, m_PlaybackMixerChannel, m_PlaybackMixerMuteOnPowerOff, /* force = */ true);
    }
    return true;
}



bool InternetRadio::noticeSoundStreamClosed(SoundStreamID id)
{
    if (id == m_SoundStreamSourceID || id == m_SoundStreamSinkID) {
        powerOff();
        return true;
    } else {
        return false;
    }
}


bool InternetRadio::noticeSoundStreamSourceRedirected(SoundStreamID oldID, SoundStreamID newID)
{
    if (m_SoundStreamSourceID == oldID) {
        m_SoundStreamSourceID = newID;
        notifyCurrentSoundStreamSourceIDChanged(m_SoundStreamSourceID);
        return true;
    } else {
        return false;
    }
}



bool InternetRadio::noticeSoundStreamSinkRedirected(SoundStreamID oldID, SoundStreamID newID)
{
    if (m_SoundStreamSinkID == oldID) {
        m_SoundStreamSinkID = newID;
        notifyCurrentSoundStreamSinkIDChanged(m_SoundStreamSinkID);
        return true;
    } else {
        return false;
    }
}



// IRadioDevice methods

bool InternetRadio::setPower (bool on)
{
    return on ? powerOn() : powerOff();
}

void InternetRadio::searchMixer(ISoundStreamClient **playback_mixer)
{
    if (playback_mixer) {
        *playback_mixer = getSoundStreamClientWithID(m_PlaybackMixerID);
        if (!*playback_mixer) {
            QList<ISoundStreamClient*> playback_mixers = queryPlaybackMixers();
            if (!playback_mixers.isEmpty())
                *playback_mixer = playback_mixers.first();
        }
    }
}


bool InternetRadio::powerOn ()
{
    if (isPowerOn())
        return true;

    radio_init();

    if (isPowerOn()) {
        ISoundStreamClient *playback_mixer = NULL;

        searchMixer(&playback_mixer);

        if (playback_mixer)
            playback_mixer->preparePlayback(m_SoundStreamSinkID, m_PlaybackMixerChannel, true, false);

        sendStartPlayback(m_SoundStreamSinkID);

        float tmp_vol = 0;
        queryPlaybackVolume(m_SoundStreamSinkID, tmp_vol);
        if (tmp_vol < 0.005)
            sendPlaybackVolume(m_SoundStreamSinkID, m_defaultPlaybackVolume);

        unmuteSource  (m_SoundStreamSourceID);
        sendUnmuteSink(m_SoundStreamSourceID);
        notifyPowerChanged(isPowerOn());
        notifyStationChanged(m_currentStation);
        notifyURLChanged(m_currentStation.url(), &m_currentStation);

        bool s = false;
        isStereo(m_SoundStreamSourceID, s);
        notifyStereoChanged(m_SoundStreamSourceID, s);
        float sq = 1.0;
        getSignalQuality(m_SoundStreamSourceID, sq);
        notifySignalQualityChanged(m_SoundStreamSourceID, sq);

    }

    return true;
}


bool InternetRadio::powerOff ()
{
    if (! isPowerOn())
        return true;

    queryPlaybackVolume(m_SoundStreamSinkID, m_defaultPlaybackVolume);
    if (m_PlaybackMixerMuteOnPowerOff) {
      sendMuteSink(m_SoundStreamSourceID);
    }
    muteSource  (m_SoundStreamSourceID);
    radio_done();

    sendStopRecording(m_SoundStreamSinkID);
    sendStopPlayback (m_SoundStreamSinkID);
    sendStopCapture  (m_SoundStreamSinkID);

    SoundStreamID oldSourceID = m_SoundStreamSourceID;
    SoundStreamID oldSinkID   = m_SoundStreamSinkID;
    m_SoundStreamSourceID     = createNewSoundStream(m_SoundStreamSourceID, false);
    m_SoundStreamSinkID       = m_SoundStreamSourceID;
    closeSoundStream (oldSourceID);
    closeSoundStream (oldSinkID);

    notifySoundStreamCreated(m_SoundStreamSourceID);
    notifyCurrentSoundStreamSinkIDChanged  (m_SoundStreamSinkID);
    notifyCurrentSoundStreamSourceIDChanged(m_SoundStreamSourceID);

    if (isPowerOff()) {
        notifyPowerChanged(false);
    }

    updateRDSState      (false);
    updateRDSStationName(QString());
    updateRDSRadioText  (QString());

    bool s = false;
    isStereo(m_SoundStreamSourceID, s);
    notifyStereoChanged(m_SoundStreamSourceID, s);
    float sq = 1.0;
    getSignalQuality(m_SoundStreamSourceID, sq);
    notifySignalQualityChanged(m_SoundStreamSourceID, sq);

    return true;
}


bool InternetRadio::activateStation(const RadioStation &rs)
{
    const InternetRadioStation *irs = dynamic_cast<const InternetRadioStation*>(&rs);
    if (irs == NULL)
        return false;

    if (setURL(irs->url(), irs)) {
        m_currentStation = *irs;

        if (irs->initialVolume() > 0)
            setPlaybackVolume(m_SoundStreamSinkID, irs->initialVolume());

        return true;
    }

    return false;
}



bool InternetRadio::isPowerOn() const
{
    return m_powerOn;
}


bool InternetRadio::isPowerOff() const
{
    return !isPowerOn();
}


/*SoundStreamID InternetRadio::getSoundStreamSinkID() const
{
    return m_SoundStreamSinkID;
}


SoundStreamID InternetRadio::getSoundStreamSourceID() const
{
    return m_SoundStreamSourceID;
}

*/
const RadioStation &InternetRadio::getCurrentStation() const
{
    return m_currentStation;
}


static QString staticInternetRadioDescription;

const QString &InternetRadio::getDescription() const
{
    if (!staticInternetRadioDescription.length()) {
       staticInternetRadioDescription = i18n("Internet radio station decoder / pseudo device");
    }
    return staticInternetRadioDescription;
}


SoundStreamID InternetRadio::getCurrentSoundStreamSinkID() const
{
    return m_SoundStreamSinkID;
}

SoundStreamID InternetRadio::getCurrentSoundStreamSourceID() const
{
    return m_SoundStreamSourceID;
}

bool InternetRadio::getRDSState() const
{
    return m_RDS_visible;
}

const QString &InternetRadio::getRDSRadioText() const
{
    return m_RDS_RadioText;
}

const QString &InternetRadio::getRDSStationName() const
{
    return m_RDS_StationName;
}




// soundstreamclient stuff
// we are only reacting on sourceID requests. effect plugins etc. have to forward and/or
// buffer(timeshifter) such "properties"

bool InternetRadio::muteSource (SoundStreamID id, bool mute)
{
    if (id != m_SoundStreamSourceID)
        return false;

    if (m_muted != mute) {
        m_muted = mute;
        notifySourceMuted(id, m_muted);
        return true;
    }
    return false;
}


bool InternetRadio::unmuteSource (SoundStreamID id, bool unmute)
{
    return muteSource(id, !unmute);
}


bool InternetRadio::getSignalQuality(SoundStreamID id, float &q) const
{
    if (id != m_SoundStreamSourceID)
        return false;

    q = 1.0; // we always have good reception :)... perhaps we can later add some code that looks for skips in the sound output
    return true;
}


bool   InternetRadio::hasGoodQuality(SoundStreamID id, bool &good) const
{
    if (id != m_SoundStreamSourceID)
        return false;

    good = true;
    return true;
}


bool    InternetRadio::isStereo(SoundStreamID id, bool &s) const
{
    if (id != m_SoundStreamSourceID)
        return false;

    s = m_stereoFlag;
    return true;
}


bool    InternetRadio::isSourceMuted(SoundStreamID id, bool &m) const
{
    if (id != m_SoundStreamSourceID)
        return false;

    m = m_muted;
    return true;
}




// pure virtual members of ThreadLoggingClient
IErrorLogClient    *InternetRadio::getErrorLogClient()
{
    return this;
}


// IInternetRadioCfg methods

static inline void assignChannelIfValid(QString &dest_channel, const QString &test_channel, const QStringList &valid_channels)
{
    if (valid_channels.contains(test_channel) || !valid_channels.size()) {
        dest_channel = test_channel;
    }
}


bool  InternetRadio::setPlaybackMixer(QString soundStreamClientID, QString ch, bool muteOnPowerOff, bool force)
{
    QString old_channel           = m_PlaybackMixerChannel;
    m_PlaybackMixerID             = soundStreamClientID;
    m_PlaybackMixerMuteOnPowerOff = muteOnPowerOff;
    ISoundStreamClient *mixer    = getSoundStreamClientWithID(m_PlaybackMixerID);
    QStringList         channels = mixer ? mixer->getPlaybackChannels() : QStringList();

    if (channels.size()) {
        assignChannelIfValid(m_PlaybackMixerChannel, channels[0], channels);  // lowest priority
    }
    assignChannelIfValid(m_PlaybackMixerChannel, "PCM",       channels);
    assignChannelIfValid(m_PlaybackMixerChannel, "Wave",      channels);
    assignChannelIfValid(m_PlaybackMixerChannel, "Master",    channels);
    assignChannelIfValid(m_PlaybackMixerChannel, ch,          channels);  // highest priority

    bool change = (m_PlaybackMixerID != soundStreamClientID) || (old_channel != m_PlaybackMixerChannel) || (muteOnPowerOff != m_PlaybackMixerMuteOnPowerOff);

    if (change || force) {
        if (isPowerOn()) {
            // only send start/stop playback if we still have a direct link to the mixer
            if (m_SoundStreamSinkID == m_SoundStreamSourceID) {
                queryPlaybackVolume(m_SoundStreamSourceID, m_defaultPlaybackVolume);
                sendStopPlayback   (m_SoundStreamSourceID);
                sendReleasePlayback(m_SoundStreamSourceID);
            }
        }

        ISoundStreamClient *playback_mixer = NULL;
        searchMixer(&playback_mixer);
        if (playback_mixer)
            playback_mixer->preparePlayback(m_SoundStreamSourceID, m_PlaybackMixerChannel, true, false);

        if (isPowerOn()) {
            // only send start/stop playback if we still have a direct link to the mixer
            if (m_SoundStreamSinkID == m_SoundStreamSourceID) {
                sendStartPlayback (m_SoundStreamSourceID);
                sendPlaybackVolume(m_SoundStreamSourceID, m_defaultPlaybackVolume);
            }
        }

        if (change)
            emit sigNotifyPlaybackMixerChanged(soundStreamClientID, ch, m_PlaybackMixerMuteOnPowerOff, /* force = */ false);
    }
    return true;
}


void InternetRadio::slotNoticePlaybackMixerChanged(const QString &mixerID, const QString &channelID, bool muteOnPowerOff, bool force)
{
    setPlaybackMixer(mixerID, channelID, muteOnPowerOff, force);
}


void InternetRadio::slotBufferSettingsChanged (int inputBufSize, int outputBufSize)
{
    m_inputBufferSize  = inputBufSize;
    m_outputBufferSize = outputBufSize;
}


void InternetRadio::slotWatchdogSettingsChanged (int timeout)
{
    m_watchdogTimeout  = timeout;
    m_watchdogTimer.stop();
}


void InternetRadio::slotDecoderSettingsChanged (int probe_size, double analysis_time)
{
    m_maxStreamProbeSize   = probe_size;
    m_maxStreamAnalyzeTime = analysis_time;
}


// PluginBase methods

void   InternetRadio::saveState (KConfigGroup &config) const
{
    PluginBase::saveState(config);

    config.writeEntry("PlaybackMixerID",             m_PlaybackMixerID);
    config.writeEntry("PlaybackMixerChannel",        m_PlaybackMixerChannel);
    config.writeEntry("PlaybackMixerMuteOnPowerOff", m_PlaybackMixerMuteOnPowerOff);
    config.writeEntry("InputBufferSize",             m_inputBufferSize);
    config.writeEntry("OutputBufferSize",            m_outputBufferSize);
    config.writeEntry("WatchdogTimeout",             m_watchdogTimeout);
    config.writeEntry("defaultPlaybackVolume",       m_defaultPlaybackVolume);
    config.writeEntry("URL",                         m_currentStation.url());
    config.writeEntry("PowerOn",                     isPowerOn());
    config.writeEntry("maxStreamProbeSizeNew",       m_maxStreamProbeSize);
    config.writeEntry("maxStreamAnalyzeTimeNew",     m_maxStreamAnalyzeTime);
    config.writeEntry("maxStreamRetries",            m_maxStreamRetries);

    saveRadioDeviceID(config);
}


void   InternetRadio::restoreState (const KConfigGroup &config)
{
    PluginBase::restoreState(config);

    restoreRadioDeviceID(config);
    QString PlaybackMixerID       = config.readEntry ("PlaybackMixerID",             QString());
    QString PlaybackMixerChannel  = config.readEntry ("PlaybackMixerChannel",        "PCM");
    bool    muteOnPowerOff        = config.readEntry ("PlaybackMixerMuteOnPowerOff", false);
    m_inputBufferSize             = config.readEntry ("InputBufferSize",             128*1024);
    m_outputBufferSize            = config.readEntry ("OutputBufferSize",            512*1024);
    m_watchdogTimeout             = config.readEntry ("WatchdogTimeout",             4);
    m_defaultPlaybackVolume       = config.readEntry ("defaultPlaybackVolume",       0.5);


    m_maxStreamProbeSize          = config.readEntry ("maxStreamProbeSizeNew",       8192);
    m_maxStreamAnalyzeTime        = config.readEntry ("maxStreamAnalyzeTimeNew",     0.8);
    m_maxStreamRetries            = config.readEntry ("maxStreamRetries",            2);


    setPlaybackMixer(PlaybackMixerID, PlaybackMixerChannel, muteOnPowerOff, /* force = */ true);
    emit sigBufferSettingsChanged  (m_inputBufferSize, m_outputBufferSize);
    emit sigWatchdogSettingsChanged(m_watchdogTimeout);
    emit sigDecoderSettingsChanged (m_maxStreamProbeSize, m_maxStreamAnalyzeTime);

    setURL(config.readEntry("URL", QUrl()), NULL);
    m_restorePowerOn = config.readEntry ("PowerOn", false);

    if (isPowerOff())
        notifyPlaybackVolumeChanged(m_SoundStreamSinkID, m_defaultPlaybackVolume);
}


void InternetRadio::startPlugin()
{
    PluginBase::startPlugin();
    setPower(m_restorePowerOn);
}

ConfigPageInfo InternetRadio::createConfigurationPage()
{
    InternetRadioConfiguration *conf = new InternetRadioConfiguration(NULL, m_SoundStreamSourceID);

    QObject::connect(this, &InternetRadio::sigNotifyPlaybackMixerChanged, conf, &InternetRadioConfiguration::slotNoticePlaybackMixerChanged);
    QObject::connect(this, &InternetRadio::sigBufferSettingsChanged,      conf, &InternetRadioConfiguration::slotBufferSettingsChanged);
    QObject::connect(this, &InternetRadio::sigWatchdogSettingsChanged,    conf, &InternetRadioConfiguration::slotWatchdogSettingsChanged);
    QObject::connect(this, &InternetRadio::sigDecoderSettingsChanged,     conf, &InternetRadioConfiguration::slotDecoderSettingsChanged);

    QObject::connect(conf, &InternetRadioConfiguration::sigPlaybackMixerChanged,    this, &InternetRadio::slotNoticePlaybackMixerChanged);
    QObject::connect(conf, &InternetRadioConfiguration::sigBufferSettingsChanged,   this, &InternetRadio::slotBufferSettingsChanged);
    QObject::connect(conf, &InternetRadioConfiguration::sigWatchdogSettingsChanged, this, &InternetRadio::slotWatchdogSettingsChanged);
    QObject::connect(conf, &InternetRadioConfiguration::sigDecoderSettingsChanged,  this, &InternetRadio::slotDecoderSettingsChanged);

    return ConfigPageInfo (conf,
                           i18n("Internet Radio"),
                           i18n("Internet Radio Options"),
                           "network-wired"
                           );
}

// IRadioClient

bool InternetRadio::noticeStationsChanged(const StationList &sl)
{
    const InternetRadioStation *irs = findMatchingStation(sl);
    if (irs) {
        if (irs->stationID() == m_currentStation.stationID()) {
            m_currentStation = *irs;
            notifyStationChanged(m_currentStation);
        } else {
            QUrl oldurl = m_currentStation.url();
            m_currentStation = *irs;
            m_currentStation.setUrl(oldurl);
            notifyStationChanged(m_currentStation);
        }
    }
    return true;
}


////////////////////////////////////////
// anything else

void InternetRadio::radio_init()
{
    m_stereoFlag = false;
    freeAllBuffers(); // just to be sure;-)

    m_waitForBufferMinFill = true;
    m_powerOn              = true;

    m_playlistHandler.setPlayListUrl(m_currentStation, m_maxStreamRetries);
    m_playlistHandler.startPlaylistDownload();

    logDebug(QString("InternetRadio::radio_init"));
}



void InternetRadio::slotPlaylistLoaded(const QList<QUrl> & playlist)
{
    m_currentPlaylist = playlist;
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    startDecoderThread();
#else
    m_playlistHandler.selectNextStream(true, true);
#endif
}


void InternetRadio::slotPlaylistStreamSelected(QUrl stream)
{
#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    stopStreamReader();
    stopDecoderThread();
    startStreamReader(stream);
#endif
}


void InternetRadio::slotPlaylistError(QString /*errorMsg*/)
{
    // no extra error msg, messages have already been issued by the playlist handler
    powerOff();
}


void InternetRadio::slotPlaylistEOL()
{
    powerOff();
}


#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
void InternetRadio::startStreamReader(QUrl stream)
{
    stopStreamReader();
    if (stream.scheme().startsWith("mms")) {
        m_streamReader = new MMSXHandler();
    } else {
        m_streamReader = new IcyHttpHandler();
    }
    connect(m_streamReader, &StreamReader::sigMetaDataUpdate,        this, &InternetRadio::slotMetaDataUpdate);
    connect(m_streamReader, &StreamReader::sigError,                 this, &InternetRadio::slotStreamError);
    connect(m_streamReader, &StreamReader::sigFinished,              this, &InternetRadio::slotStreamFinished);
    connect(m_streamReader, &StreamReader::sigStarted,               this, &InternetRadio::slotStreamStarted);
    connect(m_streamReader, &StreamReader::sigUrlChanged,            this, &InternetRadio::slotInputStreamUrlChanged);
    connect(m_streamReader, &StreamReader::sigConnectionEstablished, this, &InternetRadio::slotStreamConnectionEstablished);
    connect(m_streamReader, &StreamReader::sigStreamData,            this, &InternetRadio::slotWatchdogData);

    m_streamReader->startStreamDownload(stream, m_currentStation.metaDataEncoding());
}


void InternetRadio::stopStreamReader()
{
    if (m_streamReader) {
        m_streamReader->stopStreamDownload();
        m_streamReader->disconnect(this);
        m_streamReader->deleteLater();
        m_streamReader = NULL;
    }
}

#endif


void InternetRadio::startDecoderThread()
{
    if (m_decoderThread) {
        m_decoderThread->quit();
    }
    m_decoderThread = new DecoderThread(this,
                                        m_currentStation,
#ifdef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
                                        m_currentPlaylist,
#else
                                        m_playlistHandler.currentStreamUrl(),
                                        m_streamReader,
//                                         m_streamInputBuffer,
#endif
                                        m_inputBufferSize,
                                        MAX_BUFFER_CHUNKS,
                                        m_outputBufferSize / MAX_BUFFER_CHUNKS,
                                        m_maxStreamProbeSize,
                                        m_maxStreamAnalyzeTime,
                                        m_maxStreamRetries
                                       );
    QObject::connect(m_decoderThread, &DecoderThread::finished,   this, &InternetRadio::slotDecoderThreadFinished);
    m_decoderThread->start();
}


void InternetRadio::stopDecoderThread()
{
    checkDecoderMessages();
    if (m_decoderThread && m_decoderThread->decoder()) {
        m_decoderThread->decoder()->setDone();
    }
    if (m_decoderThread) {
        m_decoderThread->quit();
        // delete m_decoderThread; // the thread will delete itself, but only when it really finished.
        m_decoderThread = NULL;
    }
}


void InternetRadio::slotDecoderThreadFinished()
{
    // to avoid orphaned thread objects, we will delete finished threads here.
    // we cannot call deleteLater from within the thread loop, since
    // deleteLate will not schedule an event in the thread but in the main loop,
    // which might cause a deletion before the thread really finished.
    checkDecoderMessages();

    DecoderThread *t = static_cast<DecoderThread*>(sender());
    if (t == m_decoderThread) {
        m_decoderThread = NULL;
        if (isPowerOn()) {
            powerOff();
        }
    }
    t->deleteLater();
}


void InternetRadio::radio_done()
{
    freeAllBuffers();
    m_powerOn    = false;
    m_stereoFlag = false;

    m_watchdogTimer.stop();
    m_playlistHandler.stopPlaylistDownload();
#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    stopStreamReader();
#endif
    stopDecoderThread();
}



bool InternetRadio::setPlaybackVolume(SoundStreamID id, float volume)
{
    if (isPowerOff() && id == m_SoundStreamSinkID) {
        m_defaultPlaybackVolume = qBound<float>(0, volume, 1);
        return true;
    } else {
        return false;
    }
}


bool InternetRadio::getPlaybackVolume(SoundStreamID id, float &volume) const
{
    if (isPowerOff() && id == m_SoundStreamSinkID) {
        volume = m_defaultPlaybackVolume;
        return true;
    } else {
        return false;
    }
}



bool InternetRadio::getSoundStreamDescription(SoundStreamID id, QString &descr) const
{
    if (id == m_SoundStreamSourceID) {
        descr = name() + " - " + m_currentStation.name();
        return true;
    }
    else {
        return false;
    }
}


bool InternetRadio::getSoundStreamRadioStation(SoundStreamID id, const RadioStation *&rs) const
{
    if (id == m_SoundStreamSourceID) {
        rs = &m_currentStation;
        return true;
    }
    else {
        return false;
    }
}


bool InternetRadio::enumerateSourceSoundStreams(QMap<QString, SoundStreamID> &list) const
{
    if (m_SoundStreamSourceID.isValid()) {
        QString tmp = QString();
        getSoundStreamDescription(m_SoundStreamSourceID, tmp);
        list[tmp] = m_SoundStreamSourceID;
        return true;
    }
    return false;
}





void InternetRadio::updateRDSState      (bool enabled)
{
    if (enabled != m_RDS_visible) {
        m_RDS_visible = enabled;
        notifyRDSStateChanged(m_RDS_visible);
    }
}

void InternetRadio::updateRDSStationName(const QString &s)
{
    if (m_RDS_StationName != s) {
        m_RDS_StationName = s;
        notifyRDSStationNameChanged(m_RDS_StationName);
    }
}

void InternetRadio::updateRDSRadioText  (const QString &s)
{
    if (m_RDS_RadioText != s) {
        m_RDS_RadioText = s;
        notifyRDSRadioTextChanged(m_RDS_RadioText);
    }
}




bool InternetRadio::noticeReadyForPlaybackData(SoundStreamID id, size_t free_size)
{
    if (!id.isValid() || id != m_SoundStreamSourceID)
        return false;

    if (!checkDecoderMessages()) {
        return false;
    }

    int min_size = m_waitForBufferMinFill ? MIN_BUFFER_CHUNKS4PLAYBACK : 1;

    if (!m_decoderThread || !m_decoderThread->decoder())
        return false;

    int n_bufs = m_decoderThread->decoder()->availableBuffers();

    // printf("inet radio PLAY: n_bufs = %i, min_size = %i\n", n_bufs, min_size);
    if (n_bufs < min_size) {
        // m_waitForBufferMinFill = true;
//         logDebug(QString("InternetRadio::noticeReadyForPlaybackData: SKIP: min_count = %1, buf_count = %2").arg(min_size).arg(n_bufs));
        return false;
    }

    m_waitForBufferMinFill = false;

    size_t consumed_size = SIZE_T_DONT_CARE;
    while ((n_bufs = m_decoderThread->decoder()->availableBuffers()) && (free_size > 0) && (consumed_size != 0)) {

        DataBuffer         &buf           = m_decoderThread->decoder()->getFirstBuffer();
        QByteArray          data          = buf.remainingData();
        size_t              dataSize      = data.size();
        size_t              size          = qMin(dataSize, free_size);
                            consumed_size = SIZE_T_DONT_CARE;
        const SoundMetaData &md           = buf.metaData();
        const SoundFormat   &sf           = buf.soundFormat();

        if (m_stereoFlag != (sf.m_Channels >= 2)) {
            m_stereoFlag = (sf.m_Channels >= 2);
            notifyStereoChanged(m_SoundStreamSourceID, m_stereoFlag);
        }

//         logDebug(QString("InternetRadio::noticeReadyForPlaybackData: PLAY: buf_size = %1, min_count = %2, buf_count = %3").arg(size).arg(min_size).arg(n_bufs));

        notifySoundStreamData(id, sf, data.data(), size, consumed_size, md);

        if (consumed_size == SIZE_T_DONT_CARE) {
            consumed_size = size;
        }
        // printf("inet radio PLAY: buf_size = %zi, min_count = %i, buf_count = %i, free_size=%zi, avail_size=%zi\n", size, min_size, n_bufs, free_size, dataSize);
        // printf("     consumed: %zi\n", consumed_size);
        free_size -= consumed_size;
        buf.addProcessedSize(consumed_size);
        if (buf.remainingSize() <= 0) {
            m_decoderThread->decoder()->popFirstBuffer();
        }
    }
    return true;
}


bool InternetRadio::checkDecoderMessages()
{
    bool errorsDetected = !checkLogs(m_decoderThread ? m_decoderThread->decoder() : NULL, m_i18nLogPrefix, /*resetLogs = */ true);
    if (errorsDetected) {
        powerOff();
    }
    return !errorsDetected;
}



void InternetRadio::freeAllBuffers()
{
    if (m_decoderThread && m_decoderThread->decoder()) {
        m_decoderThread->decoder()->flushBuffers();
    }
}




bool InternetRadio::setURL(const QUrl &url, const InternetRadioStation *rs)
{
    QUrl oldurl = m_currentStation.url();

    if (rs) {
        m_currentStation = *rs;
    } else {
        m_currentStation                = InternetRadioStation(url, "", "");
        const InternetRadioStation *irs = findMatchingStation(queryStations());
        if (irs) {
            m_currentStation = *irs;
            m_currentStation.setUrl(url);
        }
    }

    if (oldurl != url) {
        if (isPowerOn()) {
            // with pulse audio, latency steadily increases if not powered off/on
            //radio_done();
            //radio_init();
            powerOff();
            powerOn();
        }
        notifyURLChanged(url, &m_currentStation);
    }

    bool s = false;
    isStereo(m_SoundStreamSourceID, s);
    notifyStereoChanged(m_SoundStreamSourceID, s);
    float sq = 1.0;
    getSignalQuality(m_SoundStreamSourceID, sq);
    notifySignalQualityChanged(m_SoundStreamSourceID, sq);

    notifyStationChanged(m_currentStation);
    notifySoundStreamChanged(m_SoundStreamSourceID);

    return true;
}

const QUrl &InternetRadio::getURL() const
{
    return m_currentStation.url();
}



// bool InternetRadio::prepareCapture(SoundStreamID id, const QString &channel)
// {
//     if (id.isValid() && m_SoundStreamID == id) {
//         return true;
//     }
//     return false;
// }

bool InternetRadio::startCaptureWithFormat(SoundStreamID      id,
                                           const SoundFormat &/*proposed_format*/,
                                           SoundFormat       &real_format,
                                           bool /*force_format*/)
{
    if (id != m_SoundStreamSourceID)
        return false;

    if(!isPowerOn()) {
        powerOn();
    }

    for (int i = 0; i < 100 && isPowerOn() && !(m_decoderThread && m_decoderThread->decoder() && m_decoderThread->decoder()->initDone()); ++i) {
        QEventLoop loop;
        QTimer::singleShot(200, &loop, &QEventLoop::quit);
        loop.exec();
    } // wait max 20 secs*/
    if (m_decoderThread && m_decoderThread->decoder() && m_decoderThread->decoder()->initDone()) {
        real_format = m_decoderThread->decoder()->soundFormat();
        return true;
    } else {
        return false;
    }
}



const InternetRadioStation *InternetRadio::findMatchingStation(const StationList &sl) const
{
    for (StationList::const_iterator it = sl.begin(); it != sl.end(); ++it) {
        const InternetRadioStation *irs = dynamic_cast<const InternetRadioStation *>(*it);
        if (irs && irs->url() == m_currentStation.url()) {
            return irs;
        }
    }
    return NULL;
}







void InternetRadio::slotMetaDataUpdate(KIO::MetaData metadata)
{
    if (isPowerOn() && metadata.contains("StreamTitle")) {
        QString title = metadata["StreamTitle"];
        updateRDSRadioText(title);
        updateRDSState(true);
    }
}


// #define  DEBUG_TEST_WATCHDOG

void InternetRadio::slotWatchdogData(QByteArray data)
{
#ifdef DEBUG_TEST_WATCHDOG
    bool reset = data.size() > 0 && !m_watchdogTimer.isActive();
#else
    bool reset = data.size() > 0;
#endif
    if (isPowerOn() && reset) {
        m_watchdogTimer.stop();
        if (m_watchdogTimeout > 0) {
            m_watchdogTimer.setSingleShot(true);
            m_watchdogTimer.start(m_watchdogTimeout * 1000);
        }
    }
}


void InternetRadio::slotWatchdogTimeout()
{
    if (isPowerOn() && !m_watchdogHandlerInService) {
        m_watchdogHandlerInService = true;
        logWarning(i18n("Internet Radio Plugin (%1): stream data timeout (>= %2 s)", m_playlistHandler.currentStreamUrl().toString(), m_watchdogTimeout));
        m_playlistHandler.selectNextStream(false, false, false);
        m_watchdogHandlerInService = false;
    }
}



#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD


void InternetRadio::slotStreamError(QUrl /*url*/)
{
    m_playlistHandler.selectNextStream(true, true);
}


void InternetRadio::slotStreamFinished(QUrl /*url*/)
{
    if (m_watchdogTimeout) {
        slotWatchdogTimeout();
    } else {
        powerOff();
    }
}


void InternetRadio::slotStreamStarted(QUrl /*url*/)
{
    // currently ignored
}


void InternetRadio::slotStreamConnectionEstablished(QUrl /*url*/, KIO::MetaData /*metaData*/)
{
    startDecoderThread();
}


void    InternetRadio::slotInputStreamUrlChanged(QUrl /*url*/)
{
    // currently ignored
}

//
// void InternetRadio::startStreamDownload()
// {
//     m_randStreamIdxOffset      = rint((m_currentPlaylist.size() - 1) * (float)rand() / (float)RAND_MAX);
//     m_currentStreamIdx         = 0;
//     m_currentStreamRetriesLeft = m_currentStreamRetriesMax;
//
//     tryNextStream();
// }
//
//
// void InternetRadio::tryNextStream()
// {
//     do {
//         if (--m_currentStreamRetriesLeft < 0) {
//             ++m_currentStreamIdx;
//             m_currentStreamRetriesLeft = m_currentStreamRetriesMax;
//         }
//         if (isPowerOn() && m_currentStreamIdx < m_currentPlaylist.size()) {
//             m_currentStreamUrl = m_currentPlaylist[(m_currentStreamIdx + m_randStreamIdxOffset) % m_currentPlaylist.size()];
//
//             // start download job
//             stopStreamDownload();
//             logDebug(i18n("opening stream %1", m_currentStreamUrl.toString()));
//
//             m_streamJob = KIO::get(m_currentStreamUrl, KIO::NoReload, KIO::HideProgressInfo);
//             if (m_streamJob) {
//                 QObject::connect(m_streamJob, &KIO::TransferJob::data,   this, &InternetRadio::slotStreamData);
//                 QObject::connect(m_streamJob, &KIO::TransferJob::result, this, &InternetRadio::slotStreamDone);
//                 m_streamJob->start();
//                 if (m_streamJob->error()) {
//                     logError(i18n("Failed to start stream download of %1: %2", m_currentStreamUrl.toString(), m_streamJob->errorString()));
//                     stopStreamDownload();
//                 }
//             } else {
//                 logError(i18n("Failed to start stream download of %1: KIO::get returned NULL pointer", m_currentStreamUrl.toString()));
//                 stopStreamDownload();
//             }
//         } else {
//             logError(i18n("Failed to start any stream of %1", m_currentStation.longName()));
//             powerOff();
//         }
//     } while (isPowerOn() && !m_streamJob);
// }
//
//
// void InternetRadio::stopStreamDownload()
// {
//     if (m_streamJob) {
//         QObject::disconnect(m_streamJob, &KIO::TransferJob::data,   this, &InternetRadio::slotStreamData);
//         QObject::disconnect(m_streamJob, &KIO::TransferJob::result, this, &InternetRadio::slotStreamDone);
//         m_streamJob->kill();
//         m_streamJob = NULL;
//     }
// }
//
//
// void InternetRadio::slotStreamData(KIO::Job *job, const QByteArray &data)
// {
//     if (m_streamJob == job) {
// //         logDebug(QString("stream data: %1 bytes").arg(data.size()));
//         if (m_decoderThread && m_decoderThread->decoder()) {
//             bool isfull = false;
//             m_decoderThread->decoder()->writeInputBuffer(data, isfull, m_currentStreamUrl);
//             if (isfull) {
//                 m_streamJob->suspend();
//             }
//         }
//     }
// }
//
//
// void InternetRadio::slotStreamContinue()
// {
//     if (m_streamJob) {
//         m_streamJob->resume();
//     }
// }
//
//
// void InternetRadio::slotStreamDone(KJob *job)
// {
//     if (m_streamJob == job) {
//         bool err = false;
//         if (m_streamJob->error()) {
//             logError(i18n("Failed to load stream data for %1: %2", m_currentStreamUrl.toString(), m_streamJob->errorString()));
//             err = true;
//         }
//         stopStreamDownload();
//         if (err) {
//             m_currentStreamRetriesLeft = 0;
//             tryNextStream();
//         }
//     }
//     job->deleteLater();
// }
//
#endif


