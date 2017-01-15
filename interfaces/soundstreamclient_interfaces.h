/***************************************************************************
                      sounddevice_interfaces.h  -  description
                             -------------------
    begin                : Sun Mar 21 2004
    copyright            : (C) 2004 by Martin Witte
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

/***************************************************************************
 *                                                                         *
 *   Interfaces in this header:                                            *
 *                                                                         *
 *   ISoundDevice(Client)                                                  *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_SOUNDSTREAMCLIENT_INTERFACES_H
#define KRADIO_SOUNDSTREAMCLIENT_INTERFACES_H

#include <QMap>

#include "interfaces.h"
#include "soundformat.h"
#include "soundstreamid.h"
#include "sound_metadata.h"
#include "radiostation.h"
#include "recording_template.h"

#define CALL_SNDSTR_SERVER(name, param, call) \
    inline int name param const { \
        return  iConnections.count() ? (*iConnections.begin())->name call : 0; \
    }


#define SIZE_T_DONT_CARE ((size_t)(-1))

class RadioStation;

INTERFACE(ISoundStreamServer, ISoundStreamClient)
{
friend class ISoundStreamClient;
public:
    IF_CON_DESTRUCTOR(ISoundStreamServer, -1)

    virtual void noticeConnectedI(cmplInterface *i, bool valid);
    virtual void noticeDisconnectedI(cmplInterface *i, bool valid);

    virtual QMap<QString, ISoundStreamClient *> getPlaybackClients() const;
    virtual QMap<QString, QString>              getPlaybackClientDescriptions() const;
    virtual QMap<QString, ISoundStreamClient *> getCaptureClients() const;
    virtual QMap<QString, QString>              getCaptureClientDescriptions() const;
    virtual ISoundStreamClient                 *getSoundStreamClientWithID(const QString &id) const;

ANSWERS:
    virtual QList<ISoundStreamClient*>   getPlaybackMixers() const;
    virtual QList<ISoundStreamClient*>   getCaptureMixers() const;


SENDERS:
    IF_SENDER_FINE  (  notifyPlaybackChannelsChanged, (const QString &/*client_id*/, const QStringList &)                     )
    IF_SENDER_FINE  (  notifyCaptureChannelsChanged, (const QString &/*client_id*/, const QStringList &)                      )

    IF_SENDER_FINE  (  sendPlaybackVolume, (SoundStreamID /*id*/,  float /*volume*/)                  )
    IF_SENDER_FINE  (  sendCaptureVolume, (SoundStreamID /*id*/,   float /*volume*/)                  )
    IF_SENDER_FINE  (  queryPlaybackVolume, (SoundStreamID /*id*/, float &/*volume*/)                 )
    IF_SENDER_FINE  (  queryCaptureVolume, (SoundStreamID /*id*/,  float &/*volume*/)                 )
    IF_SENDER_FINE  (  notifyPlaybackVolumeChanged, (SoundStreamID /*id*/, float /*volume*/)          )
    IF_SENDER_FINE  (  notifyCaptureVolumeChanged, (SoundStreamID /*id*/, float /*volume*/)           )

    IF_SENDER_FINE  (  notifyTrebleChanged, (SoundStreamID /*id*/, float /*v*/)                       )
    IF_SENDER_FINE  (  notifyBassChanged, (SoundStreamID /*id*/, float /*v*/)                         )
    IF_SENDER_FINE  (  notifyBalanceChanged, (SoundStreamID /*id*/, float /*v*/)                      )
    IF_SENDER_FINE  (  notifySourceMuted,         (SoundStreamID /*id*/, bool /*m*/)                  )
    IF_SENDER_FINE  (  notifySourcePlaybackMuted, (SoundStreamID /*id*/, bool /*m*/)                  )
    IF_SENDER_FINE  (  notifySinkMuted,           (SoundStreamID /*id*/, bool /*m*/)                  )
    IF_SENDER_FINE  (  notifySignalQualityChanged, (SoundStreamID /*id*/, float /*q*/)                )
    IF_SENDER_FINE  (  notifySignalQualityBoolChanged, (SoundStreamID /*id*/, bool /*good*/)          )
    IF_SENDER_FINE  (  notifySignalMinQualityChanged, (SoundStreamID /*id*/, float /*q*/)             )
    IF_SENDER_FINE  (  notifyStereoChanged, (SoundStreamID /*id*/, bool  /*s*/)                       )

    IF_SENDER_FINE  (  sendTreble,              (SoundStreamID /*id*/, float /*v*/)                    )
    IF_SENDER_FINE  (  sendBass,                (SoundStreamID /*id*/, float /*v*/)                    )
    IF_SENDER_FINE  (  sendBalance,             (SoundStreamID /*id*/, float /*v*/)                    )
    IF_SENDER_FINE  (  sendMuteSource,          (SoundStreamID /*id*/, bool mute = true)               )
    IF_SENDER_FINE  (  sendMuteSourcePlayback,  (SoundStreamID /*id*/, bool mute = true)               )
    IF_SENDER_FINE  (  sendMuteSink,            (SoundStreamID /*id*/, bool mute = true)               )
    IF_SENDER_FINE  (  sendUnmuteSource,        (SoundStreamID /*id*/, bool unmute = true)             )
    IF_SENDER_FINE  (  sendUnmuteSourcePlayback,(SoundStreamID /*id*/, bool unmute = true)             )
    IF_SENDER_FINE  (  sendUnmuteSink,          (SoundStreamID /*id*/, bool unmute = true)             )
    IF_SENDER_FINE  (  sendSignalMinQuality,    (SoundStreamID /*id*/, float /*q*/)                    )
    IF_SENDER_FINE  (  sendStereoMode,          (SoundStreamID /*id*/, StationStereoMode /*s*/)        )

    IF_SENDER_FINE  (  queryTreble,                (SoundStreamID /*id*/, float &)                     )
    IF_SENDER_FINE  (  queryBass,                  (SoundStreamID /*id*/, float &)                     )
    IF_SENDER_FINE  (  queryBalance,               (SoundStreamID /*id*/, float &)                     )
    IF_SENDER_FINE  (  querySignalQuality,         (SoundStreamID /*id*/, float &)                     )
    IF_SENDER_FINE  (  querySignalMinQuality,      (SoundStreamID /*id*/, float &)                     )
    IF_SENDER_FINE  (  queryHasGoodQuality,        (SoundStreamID /*id*/, bool &)                      )
    IF_SENDER_FINE  (  queryIsStereo,              (SoundStreamID /*id*/, bool &)                      )
    IF_SENDER_FINE  (  queryIsSourcePlaybackMuted, (SoundStreamID /*id*/, bool &)                      )
    IF_SENDER_FINE  (  queryIsSourceMuted,         (SoundStreamID /*id*/, bool &)                      )
    IF_SENDER_FINE  (  queryIsSinkMuted,           (SoundStreamID /*id*/, bool &)                      )


    // sendPreparePlayback/sendPrepareCapture don't make sense for multiple receivers
    IF_SENDER_FINE  (  sendReleasePlayback, (SoundStreamID id)                                 )
    IF_SENDER_FINE  (  sendReleaseCapture,  (SoundStreamID id)                                 )

    IF_SENDER_FINE  (  sendStartPlayback, (SoundStreamID id)                                   )
    IF_SENDER_FINE  (  sendPausePlayback, (SoundStreamID id)                                   )
    IF_SENDER_FINE  (  sendResumePlayback, (SoundStreamID id)                                  )
    IF_SENDER_FINE  (  sendStopPlayback, (SoundStreamID id)                                    )
    IF_SENDER_FINE  (  queryIsPlaybackRunning, (SoundStreamID id, bool &)                      )
    IF_SENDER_FINE  (  queryIsPlaybackPaused,  (SoundStreamID id, bool &)                      )

//    IF_SENDER_FINE  (  sendStartCapture, (SoundStreamID id)                                    )
    IF_SENDER_FINE  (  sendStartCaptureWithFormat, (SoundStreamID id,
                                   const SoundFormat &proposed_format,
                                   SoundFormat       &real_format,
                                   bool force_format = false)                                   )
    IF_SENDER_FINE  (  sendStopCapture, (SoundStreamID id)                                      )
    IF_SENDER_FINE  (  queryIsCaptureRunning, (SoundStreamID id, bool &running, SoundFormat &sf))

    // we need extra recording, in order to distinguish between plain capturing
    // (making sound data available to kradio) and writing a stream to disk  or sth similar
    IF_SENDER_FINE  (  sendStartRecording, (SoundStreamID id,
                                            const recordingTemplate_t &filenameTemplate = recordingTemplate_t())          )
    IF_SENDER_FINE  (  sendStartRecordingWithFormat, (SoundStreamID id,
                                                      const SoundFormat &proposed_format,
                                                      SoundFormat       &real_format,
                                                      const recordingTemplate_t     &filenameTemplate = recordingTemplate_t())                   )
    IF_SENDER_FINE  (  sendStopRecording, (SoundStreamID id)                                      )
    IF_SENDER_FINE  (  queryIsRecordingRunning, (SoundStreamID id, bool &running, SoundFormat &sf))

    IF_SENDER_FINE  (  querySoundStreamDescription, (SoundStreamID id, QString &descr)         )
    IF_SENDER_FINE  (  querySoundStreamRadioStation, (SoundStreamID id, const RadioStation *&rs))
    IF_SENDER_FINE  (  queryEnumerateSourceSoundStreams, (QMap<QString, SoundStreamID> &) )

    IF_SENDER_FINE  (  notifySoundStreamCreated, (SoundStreamID id)                            )
    IF_SENDER_FINE  (  notifySoundStreamClosed, (SoundStreamID id)                             )
    IF_SENDER_FINE  (  notifySoundStreamSinkRedirected, (SoundStreamID oldID, SoundStreamID newID)    )
    IF_SENDER_FINE  (  notifySoundStreamSourceRedirected, (SoundStreamID oldID, SoundStreamID newID)    )

    // e.g description or whatever changed
    IF_SENDER_FINE  (  notifySoundStreamChanged, (SoundStreamID id)                            )

    IF_SENDER_FINE  (  notifySoundStreamData, (SoundStreamID /*id*/, const SoundFormat &, const char */*data*/, size_t /*size*/, size_t &/*consumed_size*/, const SoundMetaData &/*md*/) )
    IF_SENDER_FINE  (  notifyReadyForPlaybackData, (SoundStreamID /*id*/, size_t /*size*/)           )
};


//////////////////////////////////////////////////////////////////////////////////////////////

INTERFACE(ISoundStreamClient, ISoundStreamServer)
{
public:
    ISoundStreamClient();
    virtual ~ISoundStreamClient();

    cmplInterface *getSoundStreamServer() const { return m_Server; }


    virtual void noticeConnectedSoundClient   (thisInterface *i, bool valid);
    virtual void noticeDisconnectedSoundClient(thisInterface *i, bool valid);

    virtual void noticeConnectedI   (cmplInterface *i, bool valid);
    virtual void noticeDisconnectedI(cmplInterface *i, bool valid);

// some rarely implemented functions are not pure virtual for convenience

// direct playback / capture device functions

RECEIVERS:
    IF_RECEIVER_EMPTY( preparePlayback(SoundStreamID /*id*/, const QString &/*channel*/, bool /*active_mode*/, bool /*start_immediately = false*/)   )
    IF_RECEIVER_EMPTY( prepareCapture(SoundStreamID /*id*/, const QString &/*channel*/)                                                          )
    IF_RECEIVER_EMPTY( releasePlayback(SoundStreamID /*id*/)   )
    IF_RECEIVER_EMPTY( releaseCapture(SoundStreamID /*id*/)    )

ANSWERS:
    virtual bool supportsPlayback() const  { return false; }
    virtual bool supportsCapture()  const  { return false; }

    virtual const QString &getSoundStreamClientID() const;
    virtual       QString  getSoundStreamClientDescription() const { return QString(); }

    virtual QMap<QString, ISoundStreamClient *> getPlaybackClients() const;
    virtual QMap<QString, QString>              getPlaybackClientDescriptions() const;
    virtual QMap<QString, ISoundStreamClient *> getCaptureClients() const;
    virtual QMap<QString, QString>              getCaptureClientDescriptions() const;
    virtual ISoundStreamClient                 *getSoundStreamClientWithID(const QString &id) const;

// device mixer functions

QUERIES:
    IF_QUERY ( QList<ISoundStreamClient*>   queryPlaybackMixers()     );
    IF_QUERY ( QList<ISoundStreamClient*>   queryCaptureMixers()      );


ANSWERS:
    virtual const QStringList &getPlaybackChannels() const;
    virtual const QStringList &getCaptureChannels()  const;

RECEIVERS:
    IF_RECEIVER_EMPTY(  noticePlaybackChannelsChanged(const QString & /*client_id*/, const QStringList &/*channels*/) );
    IF_RECEIVER_EMPTY(  noticeCaptureChannelsChanged (const QString & /*client_id*/, const QStringList &/*channels*/) );


RECEIVERS:
    IF_RECEIVER_EMPTY(  setPlaybackVolume(SoundStreamID /*id*/, float /*volume*/)                    )
    IF_RECEIVER_EMPTY(  setCaptureVolume(SoundStreamID /*id*/,  float /*volume*/)                    )
    IF_RECEIVER_EMPTY(  getPlaybackVolume(SoundStreamID /*id*/, float &/*volume*/) const             )
    IF_RECEIVER_EMPTY(  getCaptureVolume(SoundStreamID /*id*/,  float &/*volume*/) const             )
    IF_RECEIVER_EMPTY(  noticePlaybackVolumeChanged(SoundStreamID /*id*/, float /*volume*/)          )
    IF_RECEIVER_EMPTY(  noticeCaptureVolumeChanged(SoundStreamID /*id*/, float /*volume*/)           )

    IF_RECEIVER_EMPTY(  setTreble           (SoundStreamID /*id*/, float /*v*/)                      )
    IF_RECEIVER_EMPTY(  setBass             (SoundStreamID /*id*/, float /*v*/)                      )
    IF_RECEIVER_EMPTY(  setBalance          (SoundStreamID /*id*/, float /*v*/)                      )
    IF_RECEIVER_EMPTY(  muteSource          (SoundStreamID /*id*/, bool  /*mute*/)                   )
    IF_RECEIVER_EMPTY(  muteSourcePlayback  (SoundStreamID /*id*/, bool  /*mute*/)                   )
    IF_RECEIVER_EMPTY(  muteSink            (SoundStreamID /*id*/, bool  /*mute*/)                   )
    IF_RECEIVER_EMPTY(  unmuteSource        (SoundStreamID /*id*/, bool  /*unmute*/)                 )
    IF_RECEIVER_EMPTY(  unmuteSourcePlayback(SoundStreamID /*id*/, bool  /*unmute*/)                 )
    IF_RECEIVER_EMPTY(  unmuteSink          (SoundStreamID /*id*/, bool  /*unmute*/)                 )
    IF_RECEIVER_EMPTY(  setSignalMinQuality (SoundStreamID /*id*/, float /*q*/)                      )
    IF_RECEIVER_EMPTY(  setStereoMode       (SoundStreamID /*id*/, StationStereoMode /*m*/)          )

    IF_RECEIVER_EMPTY(  noticeTrebleChanged          (SoundStreamID /*id*/, float /*v*/)             )
    IF_RECEIVER_EMPTY(  noticeBassChanged            (SoundStreamID /*id*/, float /*v*/)             )
    IF_RECEIVER_EMPTY(  noticeBalanceChanged         (SoundStreamID /*id*/, float /*v*/)             )
    IF_RECEIVER_EMPTY(  noticeSignalQualityChanged   (SoundStreamID /*id*/, float /*q*/)             )
    IF_RECEIVER_EMPTY(  noticeSignalQualityChanged   (SoundStreamID /*id*/, bool  /*good*/)          )
    IF_RECEIVER_EMPTY(  noticeSignalMinQualityChanged(SoundStreamID /*id*/, float /*q*/)             )
    IF_RECEIVER_EMPTY(  noticeStereoChanged          (SoundStreamID /*id*/, bool  /*s*/)             )
    IF_RECEIVER_EMPTY(  noticeSourceMuted            (SoundStreamID /*id*/, bool  /*m*/)             )
    IF_RECEIVER_EMPTY(  noticeSourcePlaybackMuted    (SoundStreamID /*id*/, bool  /*m*/)             )
    IF_RECEIVER_EMPTY(  noticeSinkMuted              (SoundStreamID /*id*/, bool  /*m*/)             )

    IF_RECEIVER_EMPTY(  getTreble            (SoundStreamID /*id*/, float &/*v*/)    const           )
    IF_RECEIVER_EMPTY(  getBass              (SoundStreamID /*id*/, float &/*v*/)    const           )
    IF_RECEIVER_EMPTY(  getBalance           (SoundStreamID /*id*/, float &/*v*/)    const           )
    IF_RECEIVER_EMPTY(  isSourceMuted        (SoundStreamID /*id*/, bool  &/*m*/)    const           )
    IF_RECEIVER_EMPTY(  isSourcePlaybackMuted(SoundStreamID /*id*/, bool  &/*m*/)    const           )
    IF_RECEIVER_EMPTY(  isSinkMuted          (SoundStreamID /*id*/, bool  &/*m*/)    const           )
    IF_RECEIVER_EMPTY(  getSignalQuality     (SoundStreamID /*id*/, float &/*q*/)    const           )
    IF_RECEIVER_EMPTY(  getSignalMinQuality  (SoundStreamID /*id*/, float &/*q*/)    const           )
    IF_RECEIVER_EMPTY(  hasGoodQuality       (SoundStreamID /*id*/, bool  &/*good*/) const           )
    IF_RECEIVER_EMPTY(  isStereo             (SoundStreamID /*id*/, bool  &/*s*/)    const           )

// generic stream handling (broadcasts)

RECEIVERS:
    IF_RECEIVER_EMPTY(  startPlayback(SoundStreamID /*id*/)                                           )
    IF_RECEIVER_EMPTY(  pausePlayback(SoundStreamID /*id*/)                                           )
    IF_RECEIVER_EMPTY(  resumePlayback(SoundStreamID /*id*/)                                          )
    IF_RECEIVER_EMPTY(  stopPlayback(SoundStreamID /*id*/)                                            )
    IF_RECEIVER_EMPTY(  isPlaybackRunning(SoundStreamID /*id*/, bool &) const                         )
    IF_RECEIVER_EMPTY(  isPlaybackPaused (SoundStreamID /*id*/, bool &) const                         )

//    IF_RECEIVER_EMPTY(  startCapture(SoundStreamID /*id*/)                                            )
    IF_RECEIVER_EMPTY(  startCaptureWithFormat(SoundStreamID /*id*/,
                                     const SoundFormat &/*proposed_format*/,
                                     SoundFormat       &/*real_format*/,
                                     bool               /*force_format*/ = false)                               )
    IF_RECEIVER_EMPTY(  stopCapture(SoundStreamID /*id*/)                                                       )
    IF_RECEIVER_EMPTY(  isCaptureRunning(SoundStreamID /*id*/, bool &/*running*/, SoundFormat &/*sf*/) const    )

    IF_RECEIVER_EMPTY(  startRecording(SoundStreamID /*id*/, const recordingTemplate_t & /*template*/)               )
    IF_RECEIVER_EMPTY(  startRecordingWithFormat(SoundStreamID /*id*/,
                                     const SoundFormat &/*proposed_format*/,
                                     SoundFormat       &/*real_format*/,
                                     const recordingTemplate_t  &/*template*/
                                     )                                                                          )
    IF_RECEIVER_EMPTY(  stopRecording(SoundStreamID /*id*/)                                                     )
    IF_RECEIVER_EMPTY(  isRecordingRunning(SoundStreamID /*id*/, bool &/*running*/, SoundFormat &/*sf*/) const  )

    IF_RECEIVER_EMPTY(  getSoundStreamDescription(SoundStreamID /*id*/, QString &/*descr*/) const               )
    IF_RECEIVER_EMPTY(  getSoundStreamRadioStation(SoundStreamID /*id*/, const RadioStation *&/*rs*/) const     )

    IF_RECEIVER_EMPTY(  enumerateSourceSoundStreams(QMap<QString, SoundStreamID> &/*list*/) const                     )

    IF_RECEIVER_EMPTY(  noticeSoundStreamCreated(SoundStreamID /*id*/)                                          )
    IF_RECEIVER_EMPTY(  noticeSoundStreamClosed(SoundStreamID /*id*/)                                           )
    IF_RECEIVER_EMPTY(  noticeSoundStreamSourceRedirected(SoundStreamID /*oldID*/, SoundStreamID /*newID*/)     )
    IF_RECEIVER_EMPTY(  noticeSoundStreamSinkRedirected(SoundStreamID /*oldID*/, SoundStreamID /*newID*/)       )

    // e.g description or whatever changed
    IF_RECEIVER_EMPTY(  noticeSoundStreamChanged(SoundStreamID /*id*/)                                          )

    IF_RECEIVER_EMPTY(  noticeSoundStreamData(SoundStreamID /*id*/, const SoundFormat &, const char */*data*/, size_t /*size*/, size_t &/*consumed_size*/, const SoundMetaData &/*md*/))
    IF_RECEIVER_EMPTY(  noticeReadyForPlaybackData(SoundStreamID /*id*/, size_t /*size*/)                       )

SENDERS:
    CALL_SNDSTR_SERVER  (  notifyPlaybackChannelsChanged, (const QString &client_id, const QStringList &map), (client_id, map)                     )
    CALL_SNDSTR_SERVER  (  notifyCaptureChannelsChanged,  (const QString &client_id, const QStringList &map), (client_id, map)                     )

    CALL_SNDSTR_SERVER  (  sendPlaybackVolume, (SoundStreamID id,  float volume), (id, volume)                  )
    CALL_SNDSTR_SERVER  (  sendCaptureVolume, (SoundStreamID id,   float volume), (id, volume)                  )
    CALL_SNDSTR_SERVER  (  queryPlaybackVolume, (SoundStreamID id, float &volume), (id, volume)                 )
    CALL_SNDSTR_SERVER  (  queryCaptureVolume, (SoundStreamID id,  float &volume), (id, volume)                 )
    CALL_SNDSTR_SERVER  (  notifyPlaybackVolumeChanged, (SoundStreamID id, float volume), (id, volume)          )
    CALL_SNDSTR_SERVER  (  notifyCaptureVolumeChanged, (SoundStreamID id, float volume), (id, volume)           )

    CALL_SNDSTR_SERVER  (  notifyTrebleChanged,            (SoundStreamID id, float v),    (id, v)              )
    CALL_SNDSTR_SERVER  (  notifyBassChanged,              (SoundStreamID id, float v),    (id, v)              )
    CALL_SNDSTR_SERVER  (  notifyBalanceChanged,           (SoundStreamID id, float v),    (id, v)              )
    CALL_SNDSTR_SERVER  (  notifySourceMuted,              (SoundStreamID id, bool  m),    (id, m)              )
    CALL_SNDSTR_SERVER  (  notifySourcePlaybackMuted,      (SoundStreamID id, bool  m),    (id, m)              )
    CALL_SNDSTR_SERVER  (  notifySinkMuted,                (SoundStreamID id, bool  m),    (id, m)              )
    CALL_SNDSTR_SERVER  (  notifySignalQualityChanged,     (SoundStreamID id, float q),    (id, q)              )
    CALL_SNDSTR_SERVER  (  notifySignalQualityBoolChanged, (SoundStreamID id, bool  good), (id, good)           )
    CALL_SNDSTR_SERVER  (  notifySignalMinQualityChanged,  (SoundStreamID id, float q),    (id, q)              )
    CALL_SNDSTR_SERVER  (  notifyStereoChanged,            (SoundStreamID id, bool  s),    (id, s)              )

    CALL_SNDSTR_SERVER  (  sendTreble,                 (SoundStreamID id, float v),             (id, v)         )
    CALL_SNDSTR_SERVER  (  sendBass,                   (SoundStreamID id, float v),             (id, v)         )
    CALL_SNDSTR_SERVER  (  sendBalance,                (SoundStreamID id, float v),             (id, v)         )
    CALL_SNDSTR_SERVER  (  sendMuteSource,             (SoundStreamID id, bool  mute = true),   (id, mute)      )
    CALL_SNDSTR_SERVER  (  sendMuteSourcePlayback,     (SoundStreamID id, bool  mute = true),   (id, mute)      )
    CALL_SNDSTR_SERVER  (  sendMuteSink,               (SoundStreamID id, bool  mute = true),   (id, mute)      )
    CALL_SNDSTR_SERVER  (  sendUnmuteSource,           (SoundStreamID id, bool  unmute = true), (id, unmute)    )
    CALL_SNDSTR_SERVER  (  sendUnmuteSourcePlayback,   (SoundStreamID id, bool  unmute = true), (id, unmute)    )
    CALL_SNDSTR_SERVER  (  sendUnmuteSink,             (SoundStreamID id, bool  unmute = true), (id, unmute)    )
    CALL_SNDSTR_SERVER  (  sendSignalMinQuality,       (SoundStreamID id, float q),             (id, q)         )
    CALL_SNDSTR_SERVER  (  sendStereoMode,             (SoundStreamID id, StationStereoMode s), (id, s)         )

    CALL_SNDSTR_SERVER  (  queryTreble,                (SoundStreamID id, float &v), (id, v)                    )
    CALL_SNDSTR_SERVER  (  queryBass,                  (SoundStreamID id, float &v), (id, v)                    )
    CALL_SNDSTR_SERVER  (  queryBalance,               (SoundStreamID id, float &v), (id, v)                    )
    CALL_SNDSTR_SERVER  (  querySignalQuality,         (SoundStreamID id, float &v), (id, v)                    )
    CALL_SNDSTR_SERVER  (  querySignalMinQuality,      (SoundStreamID id, float &v), (id, v)                    )
    CALL_SNDSTR_SERVER  (  queryHasGoodQuality,        (SoundStreamID id, bool  &v), (id, v)                    )
    CALL_SNDSTR_SERVER  (  queryIsStereo,              (SoundStreamID id, bool  &v), (id, v)                    )
    CALL_SNDSTR_SERVER  (  queryIsSourceMuted,         (SoundStreamID id, bool  &v), (id, v)                    )
    CALL_SNDSTR_SERVER  (  queryIsSourcePlaybackMuted, (SoundStreamID id, bool  &v), (id, v)                    )
    CALL_SNDSTR_SERVER  (  queryIsSinkMuted,           (SoundStreamID id, bool  &v), (id, v)                    )


    // sendPreparePlayback/sendPrepareCapture don't make sense for multiple receivers
    CALL_SNDSTR_SERVER  (  sendReleasePlayback, (SoundStreamID id), (id)                                    )
    CALL_SNDSTR_SERVER  (  sendReleaseCapture,  (SoundStreamID id), (id)                                    )

    CALL_SNDSTR_SERVER  (  sendStartPlayback, (SoundStreamID id), (id)                                      )
    CALL_SNDSTR_SERVER  (  sendPausePlayback, (SoundStreamID id), (id)                                      )
    CALL_SNDSTR_SERVER  (  sendResumePlayback, (SoundStreamID id), (id)                                     )
    CALL_SNDSTR_SERVER  (  sendStopPlayback, (SoundStreamID id), (id)                                       )
    CALL_SNDSTR_SERVER  (  queryIsPlaybackRunning, (SoundStreamID id, bool &b), (id, b)                     )
    CALL_SNDSTR_SERVER  (  queryIsPlaybackPaused,  (SoundStreamID id, bool &b), (id, b)                     )

//    CALL_SNDSTR_SERVER  (  sendStartCapture, (SoundStreamID id), (id)                                       )
    CALL_SNDSTR_SERVER  (  sendStartCaptureWithFormat, (SoundStreamID id,
                                   const SoundFormat &proposed_format,
                                   SoundFormat       &real_format,
                                   bool               force_format = false), (id, proposed_format, real_format, force_format)   )
    CALL_SNDSTR_SERVER  (  sendStopCapture, (SoundStreamID id), (id)                                                            )
    CALL_SNDSTR_SERVER  (  queryIsCaptureRunning, (SoundStreamID id, bool &b, SoundFormat &sf), (id, b, sf)                     )

    CALL_SNDSTR_SERVER  (  sendStartRecording, (SoundStreamID id, const recordingTemplate_t &templ = recordingTemplate_t()), (id, templ)      )
    CALL_SNDSTR_SERVER  (  sendStartRecordingWithFormat,
                                  (SoundStreamID id,
                                   const SoundFormat &proposed_format,
                                   SoundFormat       &real_format,
                                   const recordingTemplate_t &templ = recordingTemplate_t()),
                                  (id, proposed_format, real_format, templ)                           )
    CALL_SNDSTR_SERVER  (  sendStopRecording, (SoundStreamID id), (id)                                           )
    CALL_SNDSTR_SERVER  (  queryIsRecordingRunning, (SoundStreamID id, bool &b, SoundFormat &sf), (id, b, sf)    )

    CALL_SNDSTR_SERVER  (  querySoundStreamDescription, (SoundStreamID id, QString &descr), (id, descr)          )
    CALL_SNDSTR_SERVER  (  querySoundStreamRadioStation,(SoundStreamID id, const RadioStation *&rs), (id, rs)    )

    CALL_SNDSTR_SERVER  (  queryEnumerateSourceSoundStreams, (QMap<QString, SoundStreamID> &list), (list)              )

    CALL_SNDSTR_SERVER  (  notifySoundStreamCreated, (SoundStreamID id), (id)                                    )
    CALL_SNDSTR_SERVER  (  notifySoundStreamClosed, (SoundStreamID id), (id)                                     )
    CALL_SNDSTR_SERVER  (  notifySoundStreamSourceRedirected, (SoundStreamID oldID, SoundStreamID newID), (oldID, newID)    )
    CALL_SNDSTR_SERVER  (  notifySoundStreamSinkRedirected, (SoundStreamID oldID, SoundStreamID newID), (oldID, newID)    )

    // e.g description or whatever changed
    CALL_SNDSTR_SERVER  (  notifySoundStreamChanged, (SoundStreamID id), (id)                                    )

    CALL_SNDSTR_SERVER  (  notifySoundStreamData, (SoundStreamID id, const SoundFormat &f, const char *data, size_t size, size_t &consumed_size, const SoundMetaData &md), (id, f, data, size, consumed_size, md) )
    CALL_SNDSTR_SERVER  (  notifyReadyForPlaybackData, (SoundStreamID id, size_t size), (id, size)               )

protected:

    SoundStreamID  createNewSoundStream(bool notify = true) const;
    SoundStreamID  createNewSoundStream(SoundStreamID old_id, bool notify = true) const;
    void           closeSoundStream(SoundStreamID id, bool notify = true);

    static QString createNewSoundStreamClientID();
    void           setSoundStreamClientID(const QString &s);

    QString        m_SoundStreamClientID;

    cmplInterface *m_Server;
};


#endif
