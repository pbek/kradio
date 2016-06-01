/***************************************************************************
                          internet.h  -  description
                             -------------------
    begin                : Feb 2009
    copyright            : (C) 2002-2009 Ernst Martin Witte
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

#ifndef KRADIO_INTERNETRADIO_H
#define KRADIO_INTERNETRADIO_H

#include <QTimer>

#include <kio/job.h>

#include "radiodevice_interfaces.h"
#include "radio_interfaces.h"
#include "internetradio_interfaces.h"
#include "pluginbase.h"
#include "internetradiostation.h"
#include "radio_interfaces.h"
#include "soundstreamclient_interfaces.h"
#include "stream_input_buffer.h"
#include "icy_http_handler.h"
#include "playlist_handler.h"
#include "decoder_thread.h"
#include "thread-logging.h"

class DecoderThread;

class InternetRadio : public QObject,
                      public PluginBase,
                      public IRadioDevice,
                      public IRadioClient,
                      public IInternetRadio,
                      public ISoundStreamClient,
                      public ThreadLoggingClient
{
Q_OBJECT
public:
    InternetRadio (const QString &instanceID, const QString &name);
    virtual ~InternetRadio ();

    virtual bool connectI   (Interface *);
    virtual bool disconnectI(Interface *);

    virtual QString pluginClassName() const { return QString::fromLatin1("InternetRadio"); }

    // PluginBase

public:
    virtual void   saveState    (      KConfigGroup &) const;
    virtual void   restoreState (const KConfigGroup &);
    virtual void   startPlugin();

    virtual ConfigPageInfo  createConfigurationPage();

    // IRadioDevice methods

RECEIVERS:
    virtual bool setPower(bool p);
    virtual bool powerOn();
    virtual bool powerOff();
    virtual bool activateStation(const RadioStation &rs);

ANSWERS:
    virtual bool                   isPowerOn() const;
    virtual bool                   isPowerOff() const;
    virtual const RadioStation  &  getCurrentStation() const;
    virtual const QString       &  getDescription() const;
    virtual SoundStreamID          getCurrentSoundStreamSinkID() const;
    virtual SoundStreamID          getCurrentSoundStreamSourceID() const;

    virtual bool                   getRDSState      () const;
    virtual const QString       &  getRDSRadioText  () const;
    virtual const QString       &  getRDSStationName() const;

    // IRadioClient

RECEIVERS:
    bool noticePowerChanged(bool /*on*/)                         { return false; }
    bool noticeStationChanged (const RadioStation &, int /*idx*/){ return false; }
    bool noticeStationsChanged(const StationList &sl);
    bool noticePresetFileChanged(const QString &/*f*/)           { return false; }

    bool noticeRDSStateChanged      (bool  /*enabled*/)          { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)       { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/)       { return false; }

    bool noticeCurrentSoundStreamSinkIDChanged(SoundStreamID /*id*/)   { return false; }
    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/) { return false; }

    // IInternetRadio

RECEIVERS:
    bool setURL(const KUrl &url, const InternetRadioStation *rs);

ANSWERS:
    const KUrl &getURL() const;

    // ISoundStreamClient: mixer functions

RECEIVERS:
    void noticeConnectedI          (ISoundStreamServer                *s, bool pointer_valid);
    void noticeConnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid);


    bool noticeReadyForPlaybackData(SoundStreamID id, size_t free_size);

    bool noticePlaybackChannelsChanged(const QString & /*client_id*/, const QStringList &/*channels*/);

    bool startCaptureWithFormat(SoundStreamID id, const SoundFormat &proposed_format, SoundFormat &real_format, bool force_format);

    bool muteSource   (SoundStreamID, bool mute   = true);
    bool unmuteSource (SoundStreamID, bool unmute = true);

    bool getSignalQuality(SoundStreamID, float &q) const;
    bool hasGoodQuality(SoundStreamID, bool &) const;
    bool isStereo(SoundStreamID, bool &s) const;
    bool isSourceMuted(SoundStreamID, bool &m) const;

    // ISoundStreamClient: generic stream handling (broadcasts)

RECEIVERS:

    bool getSoundStreamDescription(SoundStreamID id, QString &descr) const;
    bool getSoundStreamRadioStation(SoundStreamID id, const RadioStation *&rs) const;
    bool enumerateSourceSoundStreams(QMap<QString, SoundStreamID> &list) const;

    bool noticeSoundStreamSinkRedirected(SoundStreamID oldID, SoundStreamID newID);
    bool noticeSoundStreamSourceRedirected(SoundStreamID oldID, SoundStreamID newID);
    bool noticeSoundStreamClosed(SoundStreamID id);


    // if the radio is powered off, we will handle the volume by changing m_defaultPlaybackVolume
    bool       setPlaybackVolume(SoundStreamID id, float volume);
    bool       getPlaybackVolume(SoundStreamID id, float &volume) const;

// pure virtual members of ThreadLoggingClient
protected:
    IErrorLogClient    *getErrorLogClient();

    // anything else
public:

    bool    setPlaybackMixer(QString soundStreamClientID, QString ch, bool muteOnPowerOff, bool force);

protected slots:

    void    slotNoticePlaybackMixerChanged(const QString &mixerID, const QString &channelID, bool muteOnPowerOff, bool force);
    void    slotBufferSettingsChanged     (int inputBufSize, int outputBufSize);
    void    slotWatchdogSettingsChanged   (int timeout);
    void    slotDecoderSettingsChanged    (int probe_size, double analysis_time);

    // playlist handling via PlaylistHandler object
    void    slotPlaylistLoaded(KUrl::List playlist);
    void    slotPlaylistStreamSelected(KUrl stream);
    void    slotPlaylistError(QString errorMsg);
    void    slotPlaylistEOL();

signals:

    void    sigNotifyPlaybackMixerChanged(const QString &mixerID, const QString &channelID, bool muteOnPowerOff, bool force);
    void    sigBufferSettingsChanged  (int inputBufferSize, int outputBufferSize);
    void    sigWatchdogSettingsChanged(int watchdogTimeout);
    void    sigDecoderSettingsChanged (int probe_size, double analysis_time);

protected:

    INLINE_IMPL_DEF_noticeConnectedI(IErrorLogClient);
    INLINE_IMPL_DEF_noticeConnectedI(IRadioDevice);
    INLINE_IMPL_DEF_noticeConnectedI(IRadioClient);
    INLINE_IMPL_DEF_noticeConnectedI(IInternetRadio);

protected:
    void    radio_init();  // starts thread
    void    radio_done();  // terminates thread
    void    freeAllBuffers();

    bool    checkDecoderMessages();

protected slots:
    void    slotDecoderThreadFinished();

    void    slotMetaDataUpdate(KIO::MetaData metadata);
    void    slotWatchdogData(QByteArray data);
    void    slotWatchdogTimeout();

#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    void    slotInputStreamUrlChanged(KUrl url);
    void    slotStreamError   (KUrl url);
    void    slotStreamFinished(KUrl url);
    void    slotStreamStarted (KUrl url);
    void    slotStreamConnectionEstablished(KUrl url, KIO::MetaData metaData);
#endif

protected:
    void    startDecoderThread();
    void    stopDecoderThread();

#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    void    startStreamReader(KUrl stream);
    void    stopStreamReader();
#endif

    void    searchMixer(ISoundStreamClient **playback_mixer);

    const InternetRadioStation *findMatchingStation(const StationList &sl) const;

    void    updateRDSState      (bool enabled);
    void    updateRDSStationName(const QString &s);
    void    updateRDSRadioText  (const QString &s);

protected:

    bool                          m_powerOn;
    DecoderThread                *m_decoderThread;

    InternetRadioStation          m_currentStation;
    KUrl::List                    m_currentPlaylist;
    PlaylistHandler               m_playlistHandler;
#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    StreamReader                 *m_streamReader;
#endif

    bool                          m_stereoFlag;
    bool                          m_muted;

    float                         m_defaultPlaybackVolume;

    SoundStreamID                 m_SoundStreamSourceID;
    SoundStreamID                 m_SoundStreamSinkID;
    QString                       m_PlaybackMixerID;
    QString                       m_PlaybackMixerChannel;
    bool                          m_PlaybackMixerMuteOnPowerOff;

    bool                          m_restorePowerOn;

    bool                          m_RDS_visible;
    QString                       m_RDS_StationName;
    QString                       m_RDS_RadioText;


    int                           m_maxStreamProbeSize;    // in bytes,   see DecoderThread::openAVStream
    float                         m_maxStreamAnalyzeTime;  // in seconds, see DecoderThread::openAVStream
    int                           m_maxStreamRetries;

    int                           m_inputBufferSize;
    int                           m_outputBufferSize;

    int                           m_watchdogTimeout;
    bool                          m_watchdogHandlerInService;
    QTimer                        m_watchdogTimer;

    bool                          m_waitForBufferMinFill;

    QString                       m_i18nLogPrefix;
};

#endif
