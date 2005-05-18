/***************************************************************************
                      sounddevice_interfaces.h  -  description
                             -------------------
    begin                : Sun Mar 21 2004
    copyright            : (C) 2004 by Martin Witte
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

/***************************************************************************
 *                                                                         *
 *   Interfaces in this header:                                            *
 *                                                                         *
 *   ISoundDevice(Client)                                                  *
 *                                                                         *
 ***************************************************************************/

#ifndef KRADIO_SOUNDSTREAMCLIENT_INTERFACES_H
#define KRADIO_SOUNDSTREAMCLIENT_INTERFACES_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kconfig.h>
#include <qmap.h>

#include "interfaces.h"
#include "../libkradio/soundformat.h"
#include "../libkradio/soundstreamid.h"
#include "../libkradio/sound_metadata.h"


#define CALL_SNDSTR_SERVER(name, param, call) \
    inline int name param const { \
        return  iConnections.count() ? iConnections.getFirst()->name call : 0; \
    }

class RadioStation;

INTERFACE(ISoundStreamServer, ISoundStreamClient)
{
friend class ISoundStreamClient;
public:
    IF_CON_DESTRUCTOR(ISoundStreamServer, -1)

    void noticeConnectedI(cmplInterface *i, bool valid);
    void noticeDisconnectedI(cmplInterface *i, bool valid);

    QMap<QString, ISoundStreamClient *> getPlaybackClients() const;
    QMap<QString, QString>              getPlaybackClientDescriptions() const;
    QMap<QString, ISoundStreamClient *> getCaptureClients() const;
    QMap<QString, QString>              getCaptureClientDescriptions() const;
    ISoundStreamClient                 *getSoundStreamClientWithID(const QString &id) const;

ANSWERS:
    QPtrList<ISoundStreamClient>   getPlaybackMixers() const;
    QPtrList<ISoundStreamClient>   getCaptureMixers() const;


SENDERS:
    IF_SENDER_FINE  (  notifyPlaybackChannelsChanged, (const QString &/*client_id*/, const QMap<int, QString>&)                     )
    IF_SENDER_FINE  (  notifyCaptureChannelsChanged, (const QString &/*client_id*/, const QMap<int, QString>&)                     )

    IF_SENDER_FINE  (  sendPlaybackVolume, (SoundStreamID /*id*/,  float /*volume*/)                  )
    IF_SENDER_FINE  (  sendCaptureVolume, (SoundStreamID /*id*/,   float /*volume*/)                  )
    IF_SENDER_FINE  (  queryPlaybackVolume, (SoundStreamID /*id*/, float &/*volume*/)                 )
    IF_SENDER_FINE  (  queryCaptureVolume, (SoundStreamID /*id*/,  float &/*volume*/)                 )
    IF_SENDER_FINE  (  notifyPlaybackVolumeChanged, (SoundStreamID /*id*/, float /*volume*/)          )
    IF_SENDER_FINE  (  notifyCaptureVolumeChanged, (SoundStreamID /*id*/, float /*volume*/)           )

    IF_SENDER_FINE  (  notifyTrebleChanged, (SoundStreamID /*id*/, float /*v*/)                       )
    IF_SENDER_FINE  (  notifyBassChanged, (SoundStreamID /*id*/, float /*v*/)                         )
    IF_SENDER_FINE  (  notifyBalanceChanged, (SoundStreamID /*id*/, float /*v*/)                      )
    IF_SENDER_FINE  (  notifyMuted, (SoundStreamID /*id*/, bool /*m*/)                                )
    IF_SENDER_FINE  (  notifySignalQualityChanged, (SoundStreamID /*id*/, float /*q*/)                )
    IF_SENDER_FINE  (  notifySignalQualityBoolChanged, (SoundStreamID /*id*/, bool /*good*/)              )
    IF_SENDER_FINE  (  notifySignalMinQualityChanged, (SoundStreamID /*id*/, float /*q*/)             )
    IF_SENDER_FINE  (  notifyStereoChanged, (SoundStreamID /*id*/, bool  /*s*/)                       )

    IF_SENDER_FINE  (  sendTreble,   (SoundStreamID /*id*/, float /*v*/)                              )
    IF_SENDER_FINE  (  sendBass,     (SoundStreamID /*id*/, float /*v*/)                              )
    IF_SENDER_FINE  (  sendBalance,  (SoundStreamID /*id*/, float /*v*/)                              )
    IF_SENDER_FINE  (  sendMute,  (SoundStreamID /*id*/, bool mute = true)                            )
    IF_SENDER_FINE  (  sendUnmute,  (SoundStreamID /*id*/, bool unmute = true)                        )
    IF_SENDER_FINE  (  sendSignalMinQuality,  (SoundStreamID /*id*/, float /*q*/)                     )
    IF_SENDER_FINE  (  sendStereo, (SoundStreamID /*id*/, bool /*s*/)                                 )

    IF_SENDER_FINE  (  queryTreble, (SoundStreamID /*id*/, float &)                               )
    IF_SENDER_FINE  (  queryBass, (SoundStreamID /*id*/, float &)                                 )
    IF_SENDER_FINE  (  queryBalance, (SoundStreamID /*id*/, float &)                              )
    IF_SENDER_FINE  (  querySignalQuality, (SoundStreamID /*id*/, float &)                        )
    IF_SENDER_FINE  (  querySignalMinQuality, (SoundStreamID /*id*/, float &)                     )
    IF_SENDER_FINE  (  queryHasGoodQuality, (SoundStreamID /*id*/, bool &)                        )
    IF_SENDER_FINE  (  queryIsStereo, (SoundStreamID /*id*/, bool &)                              )
    IF_SENDER_FINE  (  queryIsMuted, (SoundStreamID /*id*/, bool &)                               )


    // sendPreparePlayback/sendPrepareCapture don't make sense for multiple receivers

    IF_SENDER_FINE  (  sendStartPlayback, (SoundStreamID id)                                   )
    IF_SENDER_FINE  (  sendPausePlayback, (SoundStreamID id)                                   )
    IF_SENDER_FINE  (  sendStopPlayback, (SoundStreamID id)                                    )
    IF_SENDER_FINE  (  queryIsPlaybackRunning, (SoundStreamID id, bool &)                      )

    IF_SENDER_FINE  (  sendStartCapture, (SoundStreamID id)                                    )
    IF_SENDER_FINE  (  sendStartCaptureWithFormat, (SoundStreamID id,
                                   const SoundFormat &proposed_format,
                                   SoundFormat       &real_format,
                                   bool force_format = false)                                  )
    IF_SENDER_FINE  (  sendStopCapture, (SoundStreamID id)                                     )
    IF_SENDER_FINE  (  queryIsCaptureRunning, (SoundStreamID id, bool &)                       )

    // we need extra recording, in order to distinguish between plain capturing
    // (making sound data available to kradio) and writing a stream to disk  or sth similar
    IF_SENDER_FINE  (  sendStartRecording, (SoundStreamID id)                                  )
    IF_SENDER_FINE  (  sendStartRecordingWithFormat, (SoundStreamID id,
                                   const SoundFormat &proposed_format,
                                   SoundFormat       &real_format)                             )
    IF_SENDER_FINE  (  sendStopRecording, (SoundStreamID id)                                   )
    IF_SENDER_FINE  (  queryIsRecordingRunning, (SoundStreamID id, bool &)                     )

    IF_SENDER_FINE  (  querySoundStreamDescription, (SoundStreamID id, QString &descr)         )
    IF_SENDER_FINE  (  querySoundStreamRadioStation, (SoundStreamID id, const RadioStation *&rs))
    IF_SENDER_FINE  (  queryEnumerateSoundStreams, (QMap<QString, SoundStreamID> &) )

    IF_SENDER_FINE  (  notifySoundStreamCreated, (SoundStreamID id)                            )
    IF_SENDER_FINE  (  notifySoundStreamClosed, (SoundStreamID id)                             )
    IF_SENDER_FINE  (  notifySoundStreamRedirected, (SoundStreamID oldID, SoundStreamID newID)    )

    // e.g description or whatever changed
    IF_SENDER_FINE  (  notifySoundStreamChanged, (SoundStreamID id)                            )

    IF_SENDER_FINE  (  notifySoundStreamData, (SoundStreamID /*id*/, const SoundFormat &, const char */*data*/, unsigned /*size*/, const SoundMetaData &/*md*/) )
    IF_SENDER_FINE  (  notifyReadyForPlaybackData, (SoundStreamID /*id*/, unsigned /*size*/)           )
};


//////////////////////////////////////////////////////////////////////////////////////////////

INTERFACE(ISoundStreamClient, ISoundStreamServer)
{
public:
    ISoundStreamClient();
    virtual ~ISoundStreamClient();

    cmplInterface *getSoundStreamServer() const { return m_Server; }


    virtual void noticeConnectedSoundClient(thisInterface *i, bool valid);
    virtual void noticeDisconnectedSoundClient(thisInterface *i, bool valid);

    void noticeConnectedI(cmplInterface *i, bool valid);
    void noticeDisconnectedI(cmplInterface *i, bool valid);

// some rarely implemented functions are not pure virtual for convenience

// direct playback / capture device functions

RECEIVERS:
    IF_RECEIVER_EMPTY( preparePlayback(SoundStreamID /*id*/, int /*channel*/, bool /*active_mode*/)   )
    IF_RECEIVER_EMPTY( prepareCapture(SoundStreamID /*id*/, int /*channel*/)                      )

ANSWERS:
    virtual bool supportsPlayback() const  { return false; }
    virtual bool supportsCapture()  const  { return false; }

    virtual const QString &getSoundStreamClientID() const;
    virtual       QString  getSoundStreamClientDescription() const { return QString::null; }

    QMap<QString, ISoundStreamClient *> getPlaybackClients() const;
    QMap<QString, QString>              getPlaybackClientDescriptions() const;
    QMap<QString, ISoundStreamClient *> getCaptureClients() const;
    QMap<QString, QString>              getCaptureClientDescriptions() const;
    ISoundStreamClient                 *getSoundStreamClientWithID(const QString &id) const;

// device mixer functions

QUERIES:
    IF_QUERY ( QPtrList<ISoundStreamClient>   queryPlaybackMixers()     );
    IF_QUERY ( QPtrList<ISoundStreamClient>   queryCaptureMixers()      );


ANSWERS:
    virtual const QMap<int, QString> &getPlaybackChannels() const;
    virtual const QMap<int, QString> &getCaptureChannels()  const;

RECEIVERS:
    IF_RECEIVER_EMPTY(  noticePlaybackChannelsChanged(const QString & /*client_id*/, const QMap<int, QString> &/*channels*/) );
    IF_RECEIVER_EMPTY(  noticeCaptureChannelsChanged (const QString & /*client_id*/, const QMap<int, QString> &/*channels*/) );


RECEIVERS:
    IF_RECEIVER_EMPTY(  setPlaybackVolume(SoundStreamID /*id*/, float /*volume*/)                    )
    IF_RECEIVER_EMPTY(  setCaptureVolume(SoundStreamID /*id*/,  float /*volume*/)                    )
    IF_RECEIVER_EMPTY(  getPlaybackVolume(SoundStreamID /*id*/, float &/*volume*/) const             )
    IF_RECEIVER_EMPTY(  getCaptureVolume(SoundStreamID /*id*/,  float &/*volume*/) const             )
    IF_RECEIVER_EMPTY(  noticePlaybackVolumeChanged(SoundStreamID /*id*/, float /*volume*/)          )
    IF_RECEIVER_EMPTY(  noticeCaptureVolumeChanged(SoundStreamID /*id*/, float /*volume*/)           )

    IF_RECEIVER_EMPTY(  setTreble (SoundStreamID /*id*/, float /*v*/)                                )
    IF_RECEIVER_EMPTY(  setBass   (SoundStreamID /*id*/, float /*v*/)                                )
    IF_RECEIVER_EMPTY(  setBalance (SoundStreamID /*id*/, float /*v*/)                               )
    IF_RECEIVER_EMPTY(  mute (SoundStreamID /*id*/, bool /*mute*/)                                   )
    IF_RECEIVER_EMPTY(  unmute (SoundStreamID /*id*/, bool /*unmute*/)                               )
    IF_RECEIVER_EMPTY(  setSignalMinQuality(SoundStreamID /*id*/, float /*q*/)                       )
    IF_RECEIVER_EMPTY(  setStereo(SoundStreamID /*id*/, bool /*s*/)                                  )

    IF_RECEIVER_EMPTY(  noticeTrebleChanged(SoundStreamID /*id*/, float /*v*/)                       )
    IF_RECEIVER_EMPTY(  noticeBassChanged(SoundStreamID /*id*/, float /*v*/)                         )
    IF_RECEIVER_EMPTY(  noticeBalanceChanged(SoundStreamID /*id*/, float /*v*/)                      )
    IF_RECEIVER_EMPTY(  noticeSignalQualityChanged(SoundStreamID /*id*/, float /*q*/)                )
    IF_RECEIVER_EMPTY(  noticeSignalQualityChanged(SoundStreamID /*id*/, bool /*good*/)              )
    IF_RECEIVER_EMPTY(  noticeSignalMinQualityChanged(SoundStreamID /*id*/, float /*q*/)             )
    IF_RECEIVER_EMPTY(  noticeStereoChanged(SoundStreamID /*id*/, bool  /*s*/)                       )
    IF_RECEIVER_EMPTY(  noticeMuted(SoundStreamID /*id*/, bool /*m*/)                                )

    IF_RECEIVER_EMPTY(  getTreble (SoundStreamID /*id*/, float &/*v*/) const                         )
    IF_RECEIVER_EMPTY(  getBass   (SoundStreamID /*id*/, float &/*v*/) const                         )
    IF_RECEIVER_EMPTY(  getBalance(SoundStreamID /*id*/, float &/*v*/) const                         )
    IF_RECEIVER_EMPTY(  isMuted(SoundStreamID /*id*/, bool &/*m*/) const                             )
    IF_RECEIVER_EMPTY(  getSignalQuality(SoundStreamID /*id*/, float &/*q*/) const                   )
    IF_RECEIVER_EMPTY(  getSignalMinQuality(SoundStreamID /*id*/, float &/*q*/) const                )
    IF_RECEIVER_EMPTY(  hasGoodQuality(SoundStreamID /*id*/, bool &/*good*/) const                   )
    IF_RECEIVER_EMPTY(  isStereo(SoundStreamID /*id*/, bool &/*s*/) const                            )

// generic stream handling (broadcasts)

RECEIVERS:
    IF_RECEIVER_EMPTY(  startPlayback(SoundStreamID /*id*/)                                           )
    IF_RECEIVER_EMPTY(  pausePlayback(SoundStreamID /*id*/)                                           )
    IF_RECEIVER_EMPTY(  stopPlayback(SoundStreamID /*id*/)                                            )
    IF_RECEIVER_EMPTY(  isPlaybackRunning(SoundStreamID /*id*/, bool &) const                         )

    IF_RECEIVER_EMPTY(  startCapture(SoundStreamID /*id*/)                                            )
    IF_RECEIVER_EMPTY(  startCaptureWithFormat(SoundStreamID /*id*/,
                                     const SoundFormat &/*proposed_format*/,
                                     SoundFormat       &/*real_format*/,
                                     bool               /*force_format*/ = false)                         )
    IF_RECEIVER_EMPTY(  stopCapture(SoundStreamID /*id*/)                                             )
    IF_RECEIVER_EMPTY(  isCaptureRunning(SoundStreamID /*id*/, bool &) const                          )

    IF_RECEIVER_EMPTY(  startRecording(SoundStreamID /*id*/)                                            )
    IF_RECEIVER_EMPTY(  startRecordingWithFormat(SoundStreamID /*id*/,
                                     const SoundFormat &/*proposed_format*/,
                                     SoundFormat       &/*real_format*/)                              )
    IF_RECEIVER_EMPTY(  stopRecording(SoundStreamID /*id*/)                                             )
    IF_RECEIVER_EMPTY(  isRecordingRunning(SoundStreamID /*id*/, bool &) const                          )

    IF_RECEIVER_EMPTY(  getSoundStreamDescription(SoundStreamID /*id*/, QString &/*descr*/) const     )
    IF_RECEIVER_EMPTY(  getSoundStreamRadioStation(SoundStreamID /*id*/, const RadioStation *&/*rs*/) const  )

    IF_RECEIVER_EMPTY(  enumerateSoundStreams(QMap<QString, SoundStreamID> &/*list*/) const                   )

    IF_RECEIVER_EMPTY(  noticeSoundStreamCreated(SoundStreamID /*id*/)                                )
    IF_RECEIVER_EMPTY(  noticeSoundStreamClosed(SoundStreamID /*id*/)                                 )
    IF_RECEIVER_EMPTY(  noticeSoundStreamRedirected(SoundStreamID /*oldID*/, SoundStreamID /*newID*/)    )

    // e.g description or whatever changed
    IF_RECEIVER_EMPTY(  noticeSoundStreamChanged(SoundStreamID /*id*/)                                    )

    IF_RECEIVER_EMPTY(  noticeSoundStreamData(SoundStreamID /*id*/, const SoundFormat &, const char */*data*/, unsigned /*size*/, const SoundMetaData &/*md*/))
    IF_RECEIVER_EMPTY(  noticeReadyForPlaybackData(SoundStreamID /*id*/, unsigned /*size*/)           )

SENDERS:
    CALL_SNDSTR_SERVER  (  notifyPlaybackChannelsChanged, (const QString &client_id, const QMap<int, QString>&map), (client_id, map)                     )
    CALL_SNDSTR_SERVER  (  notifyCaptureChannelsChanged,  (const QString &client_id, const QMap<int, QString>&map), (client_id, map)                     )

    CALL_SNDSTR_SERVER  (  sendPlaybackVolume, (SoundStreamID id,  float volume), (id, volume)                  )
    CALL_SNDSTR_SERVER  (  sendCaptureVolume, (SoundStreamID id,   float volume), (id, volume)                  )
    CALL_SNDSTR_SERVER  (  queryPlaybackVolume, (SoundStreamID id, float &volume), (id, volume)                 )
    CALL_SNDSTR_SERVER  (  queryCaptureVolume, (SoundStreamID id,  float &volume), (id, volume)                 )
    CALL_SNDSTR_SERVER  (  notifyPlaybackVolumeChanged, (SoundStreamID id, float volume), (id, volume)          )
    CALL_SNDSTR_SERVER  (  notifyCaptureVolumeChanged, (SoundStreamID id, float volume), (id, volume)           )

    CALL_SNDSTR_SERVER  (  notifyTrebleChanged, (SoundStreamID id, float v), (id, v)                       )
    CALL_SNDSTR_SERVER  (  notifyBassChanged, (SoundStreamID id, float v), (id, v)                         )
    CALL_SNDSTR_SERVER  (  notifyBalanceChanged, (SoundStreamID id, float v), (id, v)                      )
    CALL_SNDSTR_SERVER  (  notifyMuted, (SoundStreamID id, bool m), (id, m)                                )
    CALL_SNDSTR_SERVER  (  notifySignalQualityChanged, (SoundStreamID id, float q), (id, q)                )
    CALL_SNDSTR_SERVER  (  notifySignalQualityBoolChanged, (SoundStreamID id, bool good), (id, good)           )
    CALL_SNDSTR_SERVER  (  notifySignalMinQualityChanged, (SoundStreamID id, float q), (id, q)             )
    CALL_SNDSTR_SERVER  (  notifyStereoChanged, (SoundStreamID id, bool  s), (id, s)                       )

    CALL_SNDSTR_SERVER  (  sendTreble  , (SoundStreamID id, float v), (id, v)                              )
    CALL_SNDSTR_SERVER  (  sendBass    , (SoundStreamID id, float v), (id, v)                              )
    CALL_SNDSTR_SERVER  (  sendBalance , (SoundStreamID id, float v), (id, v)                              )
    CALL_SNDSTR_SERVER  (  sendMute , (SoundStreamID id, bool mute = true), (id, mute)                            )
    CALL_SNDSTR_SERVER  (  sendUnmute , (SoundStreamID id, bool unmute = true), (id, unmute)                      )
    CALL_SNDSTR_SERVER  (  sendSignalMinQuality , (SoundStreamID id, float q), (id, q)                     )
    CALL_SNDSTR_SERVER  (  sendStereo, (SoundStreamID id, bool s), (id, s)                                 )

    CALL_SNDSTR_SERVER  (  queryTreble, (SoundStreamID id, float &v), (id, v)                               )
    CALL_SNDSTR_SERVER  (  queryBass, (SoundStreamID id, float &v), (id, v)                                 )
    CALL_SNDSTR_SERVER  (  queryBalance, (SoundStreamID id, float &v), (id, v)                              )
    CALL_SNDSTR_SERVER  (  querySignalQuality, (SoundStreamID id, float &v), (id, v)                        )
    CALL_SNDSTR_SERVER  (  querySignalMinQuality, (SoundStreamID id, float &v), (id, v)                     )
    CALL_SNDSTR_SERVER  (  queryHasGoodQuality, (SoundStreamID id, bool &v), (id, v)                        )
    CALL_SNDSTR_SERVER  (  queryIsStereo, (SoundStreamID id, bool &v), (id, v)                              )
    CALL_SNDSTR_SERVER  (  queryIsMuted, (SoundStreamID id, bool &v), (id, v)                               )


    // sendPreparePlayback/sendPrepareCapture don't make sense for multiple receivers

    CALL_SNDSTR_SERVER  (  sendStartPlayback, (SoundStreamID id), (id)                                   )
    CALL_SNDSTR_SERVER  (  sendPausePlayback, (SoundStreamID id), (id)                                   )
    CALL_SNDSTR_SERVER  (  sendStopPlayback, (SoundStreamID id), (id)                                    )
    CALL_SNDSTR_SERVER  (  queryIsPlaybackRunning, (SoundStreamID id, bool &b), (id, b)                    )

    CALL_SNDSTR_SERVER  (  sendStartCapture, (SoundStreamID id), (id)                                    )
    CALL_SNDSTR_SERVER  (  sendStartCaptureWithFormat, (SoundStreamID id,
                                   const SoundFormat &proposed_format,
                                   SoundFormat       &real_format,
                                   bool               force_format = false), (id, proposed_format, real_format, force_format)   )
    CALL_SNDSTR_SERVER  (  sendStopCapture, (SoundStreamID id), (id)                                     )
    CALL_SNDSTR_SERVER  (  queryIsCaptureRunning, (SoundStreamID id, bool &b), (id, b)                     )

    CALL_SNDSTR_SERVER  (  sendStartRecording, (SoundStreamID id), (id)                                    )
    CALL_SNDSTR_SERVER  (  sendStartRecordingWithFormat, (SoundStreamID id,
                                   const SoundFormat &proposed_format,
                                   SoundFormat       &real_format), (id, proposed_format, real_format)   )
    CALL_SNDSTR_SERVER  (  sendStopRecording, (SoundStreamID id), (id)                                     )
    CALL_SNDSTR_SERVER  (  queryIsRecordingRunning, (SoundStreamID id, bool &b), (id, b)                     )

    CALL_SNDSTR_SERVER  (  querySoundStreamDescription, (SoundStreamID id, QString &descr), (id, descr)  )
    CALL_SNDSTR_SERVER  (  querySoundStreamRadioStation,(SoundStreamID id, const RadioStation *&rs), (id, rs)  )

    CALL_SNDSTR_SERVER  (  queryEnumerateSoundStreams, (QMap<QString, SoundStreamID> &list), (list)              )

    CALL_SNDSTR_SERVER  (  notifySoundStreamCreated, (SoundStreamID id), (id)                            )
    CALL_SNDSTR_SERVER  (  notifySoundStreamClosed, (SoundStreamID id), (id)                             )
    CALL_SNDSTR_SERVER  (  notifySoundStreamRedirected, (SoundStreamID oldID, SoundStreamID newID), (oldID, newID)    )

    // e.g description or whatever changed
    CALL_SNDSTR_SERVER  (  notifySoundStreamChanged, (SoundStreamID id), (id)                             )

    CALL_SNDSTR_SERVER  (  notifySoundStreamData, (SoundStreamID id, const SoundFormat &f, const char *data, unsigned size, const SoundMetaData &md), (id, f, data, size, md) )
    CALL_SNDSTR_SERVER  (  notifyReadyForPlaybackData, (SoundStreamID id, unsigned size), (id, size)           )

protected:

    SoundStreamID  createNewSoundStream(bool notify = true) const;
    SoundStreamID  createNewSoundStream(SoundStreamID old_id, bool notify = true) const;
    void           closeSoundStream(SoundStreamID id, bool notify = true);

    static QString createNewSoundStreamClientID();
    void           setSoundStreamClientID(const QString &s);

    QString        m_SoundStreamClientID;

    cmplInterface  *m_Server;
};


#endif
