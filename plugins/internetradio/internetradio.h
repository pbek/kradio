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

    virtual bool connectI   (Interface *) override;
    virtual bool disconnectI(Interface *) override;

    virtual QString pluginClassName() const override { return QString::fromLatin1("InternetRadio"); }

    // PluginBase

public:
    virtual void   saveState    (      KConfigGroup &) const override;
    virtual void   restoreState (const KConfigGroup &)       override;
    virtual void   startPlugin() override;

    virtual ConfigPageInfo  createConfigurationPage() override;

    // IRadioDevice methods

RECEIVERS:
    virtual bool setPower(bool p) override;
    virtual bool powerOn ()       override;
    virtual bool powerOff()       override;
    virtual bool activateStation(const RadioStation &rs) override;

ANSWERS:
    virtual bool                   isPowerOn () const override;
    virtual bool                   isPowerOff() const override;
    virtual const RadioStation  &  getCurrentStation() const override;
    virtual const QString       &  getDescription()    const override;
    virtual SoundStreamID          getCurrentSoundStreamSinkID  () const override;
    virtual SoundStreamID          getCurrentSoundStreamSourceID() const override;

    virtual bool                   getRDSState      () const override;
    virtual const QString       &  getRDSRadioText  () const override;
    virtual const QString       &  getRDSStationName() const override;

    // IRadioClient

RECEIVERS:
    bool noticePowerChanged(bool /*on*/)                          override { return false; }
    bool noticeStationChanged (const RadioStation &, int /*idx*/) override { return false; }
    bool noticeStationsChanged(const StationList &sl)             override;
    bool noticePresetFileChanged(const QUrl &/*f*/)               override { return false; }

    bool noticeRDSStateChanged      (bool  /*enabled*/)           override { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)        override { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/)        override { return false; }

    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/) override { return false; }
    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/) override { return false; }

    // IInternetRadio

RECEIVERS:
    bool setURL(const QUrl &url, const InternetRadioStation *rs) override;

ANSWERS:
    const QUrl &getURL() const override;

    // ISoundStreamClient: mixer functions

RECEIVERS:
    void noticeConnectedI          (ISoundStreamServer                *s, bool pointer_valid) override;
    void noticeConnectedSoundClient(ISoundStreamClient::thisInterface *i, bool pointer_valid) override;


    bool noticeReadyForPlaybackData(SoundStreamID id, size_t free_size) override;

    bool noticePlaybackChannelsChanged(const QString & /*client_id*/, const QStringList &/*channels*/) override;

    bool startCaptureWithFormat(SoundStreamID id, const SoundFormat &proposed_format, SoundFormat &real_format, bool force_format) override;

    bool muteSource   (SoundStreamID, bool mute   = true) override;
    bool unmuteSource (SoundStreamID, bool unmute = true) override;

    bool getSignalQuality(SoundStreamID, float &q) const override;
    bool hasGoodQuality  (SoundStreamID, bool &)   const override;
    bool isStereo        (SoundStreamID, bool &s)  const override;
    bool isSourceMuted   (SoundStreamID, bool &m)  const override;

    // ISoundStreamClient: generic stream handling (broadcasts)

RECEIVERS:

    bool getSoundStreamDescription  (SoundStreamID id, QString &descr)          const override;
    bool getSoundStreamRadioStation (SoundStreamID id, const RadioStation *&rs) const override;
    bool enumerateSourceSoundStreams(QMap<QString, SoundStreamID> &list)        const override;

    bool noticeSoundStreamSinkRedirected  (SoundStreamID oldID, SoundStreamID newID) override;
    bool noticeSoundStreamSourceRedirected(SoundStreamID oldID, SoundStreamID newID) override;
    bool noticeSoundStreamClosed          (SoundStreamID id)                         override;


    // if the radio is powered off, we will handle the volume by changing m_defaultPlaybackVolume
    bool       setPlaybackVolume(SoundStreamID id, float volume)        override;
    bool       getPlaybackVolume(SoundStreamID id, float &volume) const override;

// pure virtual members of ThreadLoggingClient
protected:
    IErrorLogClient    *getErrorLogClient() override;

    // anything else
public:

    bool    setPlaybackMixer(QString soundStreamClientID, QString ch, bool muteOnPowerOff, bool force);

protected slots:

    void    slotNoticePlaybackMixerChanged(const QString &mixerID, const QString &channelID, bool muteOnPowerOff, bool force);
    void    slotBufferSettingsChanged     (int inputBufSize, int outputBufSize);
    void    slotWatchdogSettingsChanged   (int timeout);
    void    slotDecoderSettingsChanged    (int probe_size, double analysis_time);

    // playlist handling via PlaylistHandler object
    void    slotPlaylistLoaded(const QList<QUrl> & playlist);
    void    slotPlaylistStreamSelected(QUrl stream);
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
    void    slotInputStreamUrlChanged(QUrl url);
    void    slotStreamError   (QUrl url);
    void    slotStreamFinished(QUrl url);
    void    slotStreamStarted (QUrl url);
    void    slotStreamConnectionEstablished(QUrl url, KIO::MetaData metaData);
#endif

protected:
    void    startDecoderThread();
    void    stopDecoderThread();

#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    void    startStreamReader(QUrl stream);
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
    QList<QUrl>                   m_currentPlaylist;
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
