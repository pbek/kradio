/***************************************************************************
                     sounddevice_interfaces.cpp  -  description
                             -------------------
    begin                : Mon Mar 21 2004
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

#include "soundstreamclient_interfaces.h"

#include <QStringList>

#include "id-generator.h"

////////////////////////////////////////////

void ISoundStreamServer::noticeConnectedI(cmplInterface *i, bool pointer_valid)
{
    for (IFIterator it(iConnections.begin()); it != iConnections.end(); ++it) {
        (*it)->noticeConnectedSoundClient(i, pointer_valid);
        cmplInterface *x = (*it);
        if (x != i && pointer_valid)
            i->noticeConnectedSoundClient(x, x->isThisInterfacePointerValid());
    }
}

void ISoundStreamServer::noticeDisconnectedI(cmplInterface *i, bool pointer_valid)
{
    for (IFIterator it(iConnections.begin()); it != iConnections.end(); ++it) {
        (*it)->noticeDisconnectedSoundClient(i, pointer_valid);
        cmplInterface *x = (*it);
        if (x != i && pointer_valid)
            i->noticeDisconnectedSoundClient(x, x->isThisInterfacePointerValid());
    }
}



QList<ISoundStreamClient*> ISoundStreamServer::getPlaybackMixers() const
{
    QList<ISoundStreamClient*> tmp;
    foreach (ISoundStreamClient *client, iConnections) {
        if (client->supportsPlayback())
            tmp.append(client);
    }
    return tmp;
}

QList<ISoundStreamClient*> ISoundStreamServer::getCaptureMixers() const
{
    QList<ISoundStreamClient*> tmp;
    foreach (ISoundStreamClient *client, iConnections) {
        if (client->supportsCapture())
            tmp.append(client);
    }
    return tmp;
}

IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendPlaybackVolume, (SoundStreamID id, float volume),
                       setPlaybackVolume(id, volume)                                                             );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendCaptureVolume, (SoundStreamID id, float volume),
                       setCaptureVolume(id, volume)                                                              );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, queryPlaybackVolume, (SoundStreamID id, float &volume),
                       getPlaybackVolume(id, volume)                                                             );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, queryCaptureVolume, (SoundStreamID id,  float &volume),
                       getCaptureVolume(id, volume)                                                              );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifyPlaybackVolumeChanged, (SoundStreamID id, float volume),
                       noticePlaybackVolumeChanged(id, volume)                                                   );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifyCaptureVolumeChanged, (SoundStreamID id, float volume),
                       noticeCaptureVolumeChanged(id, volume)                                                    );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendReleasePlayback, (SoundStreamID id),
                       releasePlayback(id)                                                                         );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendReleaseCapture, (SoundStreamID id),
                       releaseCapture(id)                                                                         );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendStartPlayback, (SoundStreamID id),
                       startPlayback(id)                                                                         );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendPausePlayback, (SoundStreamID id),
                       pausePlayback(id)                                                                         );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendResumePlayback, (SoundStreamID id),
                       resumePlayback(id)                                                                         );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendStopPlayback, (SoundStreamID id),
                       stopPlayback(id)                                                                          );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, queryIsPlaybackRunning, (SoundStreamID id, bool &b),
                       isPlaybackRunning(id, b)                                                                 );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, queryIsPlaybackPaused,  (SoundStreamID id, bool &b),
                       isPlaybackPaused (id, b)                                                                 );

//IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendStartCapture, (SoundStreamID id),
//                 startCapture(id)                                                                          );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendStartCaptureWithFormat, (SoundStreamID id, const SoundFormat &proposed_format, SoundFormat &real_format, bool force_format),
                 startCaptureWithFormat(id, proposed_format, real_format, force_format)                                            );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendStopCapture, (SoundStreamID id),
                 stopCapture(id)                                                                           );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, queryIsCaptureRunning, (SoundStreamID id, bool &b, SoundFormat &sf),
                 isCaptureRunning(id, b, sf)                                                                  );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendStartRecording, (SoundStreamID id, const recordingTemplate_t &templ),
                 startRecording(id, templ)                                                                    );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendStartRecordingWithFormat, (SoundStreamID id, const SoundFormat &proposed_format, SoundFormat &real_format, const recordingTemplate_t &templ),
                 startRecordingWithFormat(id, proposed_format, real_format, templ)                            );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendStopRecording, (SoundStreamID id),
                 stopRecording(id)                                                                           );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, queryIsRecordingRunning, (SoundStreamID id, bool &b, SoundFormat &sf),
                 isRecordingRunning(id, b, sf)                                                                  );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifySoundStreamCreated, (SoundStreamID id),
                 noticeSoundStreamCreated(id)                                                              );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifySoundStreamClosed, (SoundStreamID id),
                 noticeSoundStreamClosed(id)                                                               );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifySoundStreamSourceRedirected, (SoundStreamID oldID, SoundStreamID newID),
                 noticeSoundStreamSourceRedirected(oldID, newID)                                                    );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifySoundStreamSinkRedirected, (SoundStreamID oldID, SoundStreamID newID),
                 noticeSoundStreamSinkRedirected(oldID, newID)                                                    );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifySoundStreamChanged, (SoundStreamID id),
                 noticeSoundStreamChanged(id)                                                    );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifySoundStreamData, (SoundStreamID id, const SoundFormat &format, const char *data, size_t size, size_t &consumed_size, const SoundMetaData &md),
                 noticeSoundStreamData(id, format, data, size, consumed_size, md)                                             );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifyReadyForPlaybackData, (SoundStreamID id, size_t size),
                 noticeReadyForPlaybackData(id, size)                                                      );


IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifyTrebleChanged, (SoundStreamID id, float v),
                 noticeTrebleChanged(id, v)                                                                );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifyBassChanged, (SoundStreamID id, float v),
                 noticeBassChanged(id, v)                                                                  );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifyBalanceChanged, (SoundStreamID id, float v),
                 noticeBalanceChanged(id, v)                                                               );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifySourceMuted, (SoundStreamID id, bool m),
                 noticeSourceMuted(id, m)                                                                        );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifySourcePlaybackMuted, (SoundStreamID id, bool m),
                 noticeSourcePlaybackMuted(id, m)                                                                        );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifySinkMuted, (SoundStreamID id, bool m),
                 noticeSinkMuted(id, m)                                                                        );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifySignalQualityChanged, (SoundStreamID id, float q),
                 noticeSignalQualityChanged(id, q)                                                         );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifySignalQualityBoolChanged, (SoundStreamID id, bool good),
                 noticeSignalQualityChanged(id, good)                                                      );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifySignalMinQualityChanged, (SoundStreamID id, float q),
                 noticeSignalMinQualityChanged(id, q)                                                      );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifyStereoChanged, (SoundStreamID id, bool  s),
                 noticeStereoChanged(id, s)                                                                );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendTreble  , (SoundStreamID id, float v),
                 setTreble(id, v)                                                                          );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendBass    , (SoundStreamID id, float v),
                 setBass(id, v)                                                                            );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendBalance , (SoundStreamID id, float v),
                 setBalance(id, v)                                                                         );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendMuteSource , (SoundStreamID id, bool mute),
                 muteSource(id, mute)                                                                            );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendMuteSourcePlayback, (SoundStreamID id, bool mute),
                 muteSourcePlayback(id, mute)                                                                            );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendMuteSink , (SoundStreamID id, bool mute),
                 muteSink(id, mute)                                                                            );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendUnmuteSource , (SoundStreamID id, bool unmute),
                 unmuteSource(id, unmute)                                                                        );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendUnmuteSourcePlayback , (SoundStreamID id, bool unmute),
                 unmuteSourcePlayback(id, unmute)                                                                        );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendUnmuteSink , (SoundStreamID id, bool unmute),
                 unmuteSink(id, unmute)                                                                        );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendSignalMinQuality , (SoundStreamID id, float q),
                 setSignalMinQuality(id, q)                                                                );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendStereoMode, (SoundStreamID id, StationStereoMode s),
                 setStereoMode(id, s)                                                                      );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, queryTreble, (SoundStreamID id, float &v),
                 getTreble(id, v)                                                                          );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, queryBass, (SoundStreamID id, float &v),
                 getBass(id, v)                                                                            );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, queryBalance, (SoundStreamID id, float &v),
                 getBalance(id, v)                                                                         );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, querySignalQuality, (SoundStreamID id, float &q),
                 getSignalQuality(id, q)                                                                   );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, querySignalMinQuality, (SoundStreamID id, float &q),
                 getSignalMinQuality(id, q)                                                                );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, queryHasGoodQuality, (SoundStreamID id, bool &good),
                 hasGoodQuality(id, good)                                                                  );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, queryIsStereo, (SoundStreamID id, bool &s),
                 isStereo(id, s)                                                                           );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, queryIsSourceMuted, (SoundStreamID id, bool &m),
                 isSourceMuted(id, m)                                                                            );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, queryIsSourcePlaybackMuted, (SoundStreamID id, bool &m),
                 isSourcePlaybackMuted(id, m)                                                                            );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, queryIsSinkMuted, (SoundStreamID id, bool &m),
                 isSinkMuted(id, m)                                                                            );


IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifyPlaybackChannelsChanged, (const QString &client_id, const QStringList &map),
                 noticePlaybackChannelsChanged(client_id, map)                   );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifyCaptureChannelsChanged,  (const QString &client_id, const QStringList &map),
                 noticeCaptureChannelsChanged (client_id, map)                   );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, querySoundStreamDescription, (SoundStreamID id, QString &descr),
                 getSoundStreamDescription(id, descr)                                           );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, querySoundStreamRadioStation, (SoundStreamID id, const RadioStation *&rs),
                 getSoundStreamRadioStation(id, rs)                                             );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, queryEnumerateSourceSoundStreams, (QMap<QString, SoundStreamID> &list),
                 enumerateSourceSoundStreams(list)                                             );




////////////////////////////////////////////


ISoundStreamClient::ISoundStreamClient()
    : BaseClass(1),
      m_Server(NULL)
{
    setSoundStreamClientID(createNewSoundStreamClientID());
}


ISoundStreamClient::~ISoundStreamClient()
{
}

QString ISoundStreamClient::createNewSoundStreamClientID()
{
    return generateRandomID(64);
}



void ISoundStreamClient::setSoundStreamClientID(const QString &s)
{
    ISoundStreamServer *server = getSoundStreamServer();
    if (server)
        server->noticeDisconnectedI(this, true);
    m_SoundStreamClientID = s;
    if (server)
        server->noticeConnectedI(this, true);
}


const QString &ISoundStreamClient::getSoundStreamClientID() const
{
    return m_SoundStreamClientID;
}



void ISoundStreamClient::noticeConnectedI(cmplInterface *i, bool valid)
{
    if (valid && i)
        m_Server = i;
}

void ISoundStreamClient::noticeDisconnectedI(cmplInterface *i, bool /*valid*/)
{
    if (i == m_Server) {
        m_Server = NULL;
    }
}


SoundStreamID ISoundStreamClient::createNewSoundStream(bool notify) const
{
    SoundStreamID x = SoundStreamID::createNewID();
    if (iConnections.count() && notify)
        (*iConnections.begin())->notifySoundStreamCreated(x);
    return x;
}


SoundStreamID ISoundStreamClient::createNewSoundStream(SoundStreamID old_id, bool notify) const
{
    SoundStreamID x = SoundStreamID::createNewID(old_id);
    if (iConnections.count() && notify)
        (*iConnections.begin())->notifySoundStreamCreated(x);
    return x;
}


void  ISoundStreamClient::closeSoundStream(SoundStreamID id, bool notify)
{
    if (iConnections.count() && notify)
        (*iConnections.begin())->notifySoundStreamClosed(id);
}


static const QStringList emptyList;

const QStringList &ISoundStreamClient::getPlaybackChannels() const
{
    return emptyList;
}


const QStringList &ISoundStreamClient::getCaptureChannels()  const
{
    return emptyList;
}


static const QMap<QString, ISoundStreamClient *> emptyClientMap;
QMap<QString, ISoundStreamClient *> ISoundStreamClient::getPlaybackClients() const
{
    return iConnections.count() ? (*iConnections.begin())->getPlaybackClients() : emptyClientMap;
}

QMap<QString, ISoundStreamClient *> ISoundStreamServer::getPlaybackClients() const
{
    QMap<QString, ISoundStreamClient *> map;

    foreach (ISoundStreamClient *client, iConnections) {
        if (client->supportsPlayback())
            map.insert(client->getSoundStreamClientID(), client);
    }
    return map;
}



static const QMap<QString, QString> emptyClientDescrMap;
QMap<QString, QString> ISoundStreamClient::getPlaybackClientDescriptions() const
{
    return iConnections.count() ? (*iConnections.begin())->getPlaybackClientDescriptions() : emptyClientDescrMap;
}

QMap<QString, QString> ISoundStreamServer::getPlaybackClientDescriptions() const
{
    QMap<QString, QString> map;
    foreach (ISoundStreamClient *client, iConnections) {
        if (client->supportsPlayback())
            map.insert(client->getSoundStreamClientID(), client->getSoundStreamClientDescription());
    }
    return map;
}




QMap<QString, ISoundStreamClient *> ISoundStreamClient::getCaptureClients() const
{
    return iConnections.count() ? (*iConnections.begin())->getCaptureClients() : emptyClientMap;
}


QMap<QString, ISoundStreamClient *> ISoundStreamServer::getCaptureClients() const
{
    QMap<QString, ISoundStreamClient *> map;
    foreach (ISoundStreamClient *client, iConnections) {
        if (client->supportsCapture())
            map.insert(client->getSoundStreamClientID(), client);
    }
    return map;
}




QMap<QString, QString> ISoundStreamClient::getCaptureClientDescriptions() const
{
    return iConnections.count() ? (*iConnections.begin())->getCaptureClientDescriptions() : emptyClientDescrMap;
}

QMap<QString, QString> ISoundStreamServer::getCaptureClientDescriptions() const
{
    QMap<QString, QString> map;
    foreach (ISoundStreamClient *client, iConnections) {
        if (client->supportsCapture())
            map.insert(client->getSoundStreamClientID(), client->getSoundStreamClientDescription());
    }
    return map;
}





ISoundStreamClient *ISoundStreamClient::getSoundStreamClientWithID(const QString &search_id) const
{
    return iConnections.count() ? (*iConnections.begin())->getSoundStreamClientWithID(search_id) : NULL;
}

ISoundStreamClient *ISoundStreamServer::getSoundStreamClientWithID(const QString &search_id) const
{
    foreach (ISoundStreamClient *client, iConnections) {
        const QString &id = client->getSoundStreamClientID();
        if (id == search_id)
            return client;
    }
    return NULL;
}

void ISoundStreamClient::noticeConnectedSoundClient(thisInterface */*i*/, bool /*pointer_valid*/)
{
}

void ISoundStreamClient::noticeDisconnectedSoundClient(thisInterface */*i*/, bool /*pointer_valid*/)
{
}

static const QList<ISoundStreamClient*> emptyClientList;
IF_IMPL_QUERY   (  QList<ISoundStreamClient*> ISoundStreamClient::queryPlaybackMixers(),
                   getPlaybackMixers(),
                   emptyClientList     );

IF_IMPL_QUERY   (  QList<ISoundStreamClient*> ISoundStreamClient::queryCaptureMixers(),
                   getPlaybackMixers(),
                   emptyClientList     );
