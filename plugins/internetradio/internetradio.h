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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <QtCore/QTimer>

#include <kio/job.h>

#include "radiodevice_interfaces.h"
#include "radio_interfaces.h"
#include "internetradio_interfaces.h"
#include "pluginbase.h"
#include "internetradiostation.h"
#include "radio_interfaces.h"
#include "soundstreamclient_interfaces.h"

class DecoderThread;

class InternetRadio : public QObject,
                      public PluginBase,
                      public IRadioDevice,
                      public IRadioClient,
                      public IInternetRadio,
                      public ISoundStreamClient/*,*/
//                       public IV4LCfg
{
Q_OBJECT
public:
    InternetRadio (const QString &instanceID, const QString &name);
    virtual ~InternetRadio ();

    virtual bool connectI   (Interface *);
    virtual bool disconnectI(Interface *);

    virtual QString pluginClassName() const { return "InternetRadio"; }

//     virtual const QString &name() const { return PluginBase::name(); }
//     virtual       QString &name()       { return PluginBase::name(); }

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


    // anything else
public:

    bool  setPlaybackMixer(const QString &soundStreamClientID, const QString &ch, bool muteOnPowerOff, bool force);

protected slots:

    bool event(QEvent *e);
    void slotNoticePlaybackMixerChanged(const QString &mixerID, const QString &channelID, bool muteOnPowerOff, bool force);
//     void slotMimetypeResult(KIO::Job *job, const QString &type);

signals:

    void sigNotifyPlaybackMixerChanged(const QString &mixerID, const QString &channelID, bool muteOnPowerOff, bool force);


protected:

    INLINE_IMPL_DEF_noticeConnectedI(IErrorLogClient);
    INLINE_IMPL_DEF_noticeConnectedI(IRadioDevice);
    INLINE_IMPL_DEF_noticeConnectedI(IRadioClient);
    INLINE_IMPL_DEF_noticeConnectedI(IInternetRadio);

protected:
    void    radio_init();  // starts thread
    void    radio_done();  // terminates thread
    void    freeAllBuffers();


protected slots:
    void    slotPlaylistData(KIO::Job *job, const QByteArray &data);
    void    slotPlaylistLoadDone(KJob *job);


#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    void    slotStreamData(KIO::Job *job, const QByteArray &data);
    void    slotStreamDone(KJob *job);
    void    slotStreamContinue();
#endif

protected:
    void    loadPlaylistStopJob();
    void    loadPlaylistStartJob();
    QString getPlaylistClass();
    void    interpretePlaylistData(const QByteArray &a);
    void    interpretePlaylistLSC(const QByteArray &a);
    void    interpretePlaylistM3U(const QByteArray &playlistData);
    void    interpretePlaylistPLS(const QByteArray &playlistData);
    void    interpretePlaylistASX(const QByteArray &xmlData);


#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    void    startStreamDownload();
    void    tryNextStream();
    void    stopStreamDownload();
#endif
    void    startDecoderThread();

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
#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    KUrl                          m_currentStreamUrl;
    int                           m_currentStreamIdx;
    int                           m_currentStreamRetriesMax;
    int                           m_currentStreamRetriesLeft;
    int                           m_randStreamIdxOffset;
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

    bool                          m_waitForBufferMinFill;

    QByteArray                    m_playlistData;
    KIO::TransferJob             *m_playlistJob;
#ifndef INET_RADIO_STREAM_HANDLING_BY_DECODER_THREAD
    KIO::TransferJob             *m_streamJob;
#endif
};

#endif
