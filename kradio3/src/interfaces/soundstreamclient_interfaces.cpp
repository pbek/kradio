/***************************************************************************
                     sounddevice_interfaces.cpp  -  description
                             -------------------
    begin                : Mon Mär 21 2004
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

#include "soundstreamclient_interfaces.h"

#include <unistd.h>
#include <time.h>
#include <fcntl.h>

const char *dev_urandom = "/dev/urandom";

////////////////////////////////////////////

void ISoundStreamServer::noticeConnectedI(cmplInterface *i, bool pointer_valid)
{
    for (QPtrListIterator<cmplInterface> it(iConnections); it.current(); ++it) {
        it.current()->noticeConnectedSoundClient(i, pointer_valid);
        cmplInterface *x = it.current();
        if (x != i && pointer_valid)
            i->noticeConnectedSoundClient(x, x->isThisInterfacePointerValid());
    }
}

void ISoundStreamServer::noticeDisconnectedI(cmplInterface *i, bool pointer_valid)
{
    for (QPtrListIterator<cmplInterface> it(iConnections); it.current(); ++it) {
        it.current()->noticeDisconnectedSoundClient(i, pointer_valid);
        cmplInterface *x = it.current();
        if (x != i && pointer_valid)
            i->noticeDisconnectedSoundClient(x, x->isThisInterfacePointerValid());
    }
}



QPtrList<ISoundStreamClient> ISoundStreamServer::getPlaybackMixers() const
{
    QPtrList<ISoundStreamClient> tmp;
    for (QPtrListIterator<ISoundStreamClient> it(iConnections); it.current(); ++it) {
        if (it.current()->supportsPlayback())
            tmp.append(it.current());
    }
    return tmp;
}

QPtrList<ISoundStreamClient> ISoundStreamServer::getCaptureMixers() const
{
    QPtrList<ISoundStreamClient> tmp;
    for (QPtrListIterator<ISoundStreamClient> it(iConnections); it.current(); ++it) {
        if (it.current()->supportsCapture())
            tmp.append(it.current());
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

IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendStopPlayback, (SoundStreamID id),
                 stopPlayback(id)                                                                          );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, queryIsPlaybackRunning, (SoundStreamID id, bool &b),
                 isPlaybackRunning(id, b)                                                                 );

//IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendStartCapture, (SoundStreamID id),
//                 startCapture(id)                                                                          );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendStartCaptureWithFormat, (SoundStreamID id, const SoundFormat &proposed_format, SoundFormat &real_format, bool force_format),
                 startCaptureWithFormat(id, proposed_format, real_format, force_format)                                            );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendStopCapture, (SoundStreamID id),
                 stopCapture(id)                                                                           );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, queryIsCaptureRunning, (SoundStreamID id, bool &b, SoundFormat &sf),
                 isCaptureRunning(id, b, sf)                                                                  );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendStartRecording, (SoundStreamID id),
                 startRecording(id)                                                                          );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendStartRecordingWithFormat, (SoundStreamID id, const SoundFormat &proposed_format, SoundFormat &real_format),
                 startRecordingWithFormat(id, proposed_format, real_format)                                            );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendStopRecording, (SoundStreamID id),
                 stopRecording(id)                                                                           );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, queryIsRecordingRunning, (SoundStreamID id, bool &b, SoundFormat &sf),
                 isRecordingRunning(id, b, sf)                                                                  );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifySoundStreamCreated, (SoundStreamID id),
                 noticeSoundStreamCreated(id)                                                              );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifySoundStreamClosed, (SoundStreamID id),
                 noticeSoundStreamClosed(id)                                                               );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifySoundStreamRedirected, (SoundStreamID oldID, SoundStreamID newID),
                 noticeSoundStreamRedirected(oldID, newID)                                                    );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifySoundStreamChanged, (SoundStreamID id),
                 noticeSoundStreamChanged(id)                                                    );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifySoundStreamData, (SoundStreamID id, const SoundFormat &format, const char *data, size_t size, const SoundMetaData &md),
                 noticeSoundStreamData(id, format, data, size, md)                                             );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifyReadyForPlaybackData, (SoundStreamID id, size_t size),
                 noticeReadyForPlaybackData(id, size)                                                      );


IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifyTrebleChanged, (SoundStreamID id, float v),
                 noticeTrebleChanged(id, v)                                                                );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifyBassChanged, (SoundStreamID id, float v),
                 noticeBassChanged(id, v)                                                                  );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifyBalanceChanged, (SoundStreamID id, float v),
                 noticeBalanceChanged(id, v)                                                               );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifyMuted, (SoundStreamID id, bool m),
                 noticeMuted(id, m)                                                                        );
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
IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendMute , (SoundStreamID id, bool mute),
                 mute(id, mute)                                                                            );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendUnmute , (SoundStreamID id, bool unmute),
                 unmute(id, unmute)                                                                        );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendSignalMinQuality , (SoundStreamID id, float q),
                 setSignalMinQuality(id, q)                                                                );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, sendStereo, (SoundStreamID id, bool s),
                 setStereo(id, s)                                                                          );

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
IF_IMPL_SENDER_FINE (  ISoundStreamServer, queryIsMuted, (SoundStreamID id, bool &m),
                 isMuted(id, m)                                                                            );


IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifyPlaybackChannelsChanged, (const QString &client_id, const QStringList &map),
                 noticePlaybackChannelsChanged(client_id, map)                   );
IF_IMPL_SENDER_FINE (  ISoundStreamServer, notifyCaptureChannelsChanged,  (const QString &client_id, const QStringList &map),
                 noticeCaptureChannelsChanged (client_id, map)                   );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, querySoundStreamDescription, (SoundStreamID id, QString &descr),
                 getSoundStreamDescription(id, descr)                                           );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, querySoundStreamRadioStation, (SoundStreamID id, const RadioStation *&rs),
                 getSoundStreamRadioStation(id, rs)                                             );

IF_IMPL_SENDER_FINE (  ISoundStreamServer, queryEnumerateSoundStreams, (QMap<QString, SoundStreamID> &list),
                 enumerateSoundStreams(list)                                             );




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
    const int buffersize = 32;
    unsigned char buffer[buffersize];

    QString stime, srandom = QString::null;
    stime.setNum(time(NULL));

    int fd = open (dev_urandom, O_RDONLY);
    read(fd, buffer, buffersize);
    close(fd);
    for (int i = 0; i < buffersize; ++i)
        srandom += QString().sprintf("%02X", (unsigned int)buffer[i]);

    return stime + srandom;
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
        iConnections.getFirst()->notifySoundStreamCreated(x);
    return x;
}


SoundStreamID ISoundStreamClient::createNewSoundStream(SoundStreamID old_id, bool notify) const
{
    SoundStreamID x = SoundStreamID::createNewID(old_id);
    if (iConnections.count() && notify)
        iConnections.getFirst()->notifySoundStreamCreated(x);
    return x;
}


void  ISoundStreamClient::closeSoundStream(SoundStreamID id, bool notify)
{
    if (iConnections.count() && notify)
        iConnections.getFirst()->notifySoundStreamClosed(id);
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
    return iConnections.count() ? iConnections.getFirst()->getPlaybackClients() : emptyClientMap;
}

QMap<QString, ISoundStreamClient *> ISoundStreamServer::getPlaybackClients() const
{
    QMap<QString, ISoundStreamClient *> map;
    for (QPtrListIterator<ISoundStreamClient> it(ISoundStreamServer::iConnections); it.current(); ++it) {
        if (it.current()->supportsPlayback())
            map.insert(it.current()->getSoundStreamClientID(), it.current());
    }
    return map;
}



static const QMap<QString, QString> emptyClienDescrMap;
QMap<QString, QString> ISoundStreamClient::getPlaybackClientDescriptions() const
{
    return iConnections.count() ? iConnections.getFirst()->getPlaybackClientDescriptions() : emptyClienDescrMap;
}

QMap<QString, QString> ISoundStreamServer::getPlaybackClientDescriptions() const
{
    QMap<QString, QString> map;
    for (QPtrListIterator<ISoundStreamClient> it(ISoundStreamServer::iConnections); it.current(); ++it) {
        if (it.current()->supportsPlayback())
            map.insert(it.current()->getSoundStreamClientID(), it.current()->getSoundStreamClientDescription());
    }
    return map;
}




QMap<QString, ISoundStreamClient *> ISoundStreamClient::getCaptureClients() const
{
    return iConnections.count() ? iConnections.getFirst()->getCaptureClients() : emptyClientMap;
}


QMap<QString, ISoundStreamClient *> ISoundStreamServer::getCaptureClients() const
{
    QMap<QString, ISoundStreamClient *> map;
    for (QPtrListIterator<ISoundStreamClient> it(ISoundStreamServer::iConnections); it.current(); ++it) {
        if (it.current()->supportsCapture())
            map.insert(it.current()->getSoundStreamClientID(), it.current());
    }
    return map;
}




QMap<QString, QString> ISoundStreamClient::getCaptureClientDescriptions() const
{
    return iConnections.count() ? iConnections.getFirst()->getCaptureClientDescriptions() : emptyClienDescrMap;
}

QMap<QString, QString> ISoundStreamServer::getCaptureClientDescriptions() const
{
    QMap<QString, QString> map;
    for (QPtrListIterator<ISoundStreamClient> it(ISoundStreamServer::iConnections); it.current(); ++it) {
        if (it.current()->supportsCapture())
            map.insert(it.current()->getSoundStreamClientID(), it.current()->getSoundStreamClientDescription());
    }
    return map;
}





ISoundStreamClient *ISoundStreamClient::getSoundStreamClientWithID(const QString &search_id) const
{
    return iConnections.count() ? iConnections.getFirst()->getSoundStreamClientWithID(search_id) : NULL;
}

ISoundStreamClient *ISoundStreamServer::getSoundStreamClientWithID(const QString &search_id) const
{
    for (QPtrListIterator<ISoundStreamClient> it(ISoundStreamServer::iConnections); it.current(); ++it) {
        const QString &id = it.current()->getSoundStreamClientID();
        if (id == search_id)
            return it.current();
    }
    return NULL;
}

void ISoundStreamClient::noticeConnectedSoundClient(thisInterface */*i*/, bool /*pointer_valid*/)
{
}

void ISoundStreamClient::noticeDisconnectedSoundClient(thisInterface */*i*/, bool /*pointer_valid*/)
{
}

static const QPtrList<ISoundStreamClient> emptyClientList;
IF_IMPL_QUERY   (  QPtrList<ISoundStreamClient> ISoundStreamClient::queryPlaybackMixers(),
                   getPlaybackMixers(),
                   emptyClientList     );

IF_IMPL_QUERY   (  QPtrList<ISoundStreamClient> ISoundStreamClient::queryCaptureMixers(),
                   getPlaybackMixers(),
                   emptyClientList     );
