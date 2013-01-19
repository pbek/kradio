/***************************************************************************
                          recording.h  -  description
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

#ifndef KRADIO_RECORDING_H
#define KRADIO_RECORDING_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMap>

#include "pluginbase.h"
#include "timecontrol_interfaces.h"
#include "soundstreamclient_interfaces.h"
#include "radio_interfaces.h"

#include "recording-config.h"
#include "reccfg_interfaces.h"
#include "encoder.h"

class RadioStation;
class StationList;
class QSocketNotifier;
class RecordingEncoding;
class FileRingBuffer;

class Recording : public QObject,
                  public PluginBase,
                  public IRadioClient,
                  public ISoundStreamClient,
                  public IRecCfg
{
Q_OBJECT
public:
    Recording(const QString &instanceID, const QString &name);
    ~Recording();

    virtual QString pluginClassName() const { return "Recording"; }

//     virtual const QString &name() const { return PluginBase::name(); }
//     virtual       QString &name()       { return PluginBase::name(); }

    virtual bool   connectI(Interface *i);
    virtual bool   disconnectI(Interface *i);


    bool isRecording () const;


    // PluginBase

public:
    virtual void   saveState   (      KConfigGroup &) const;
    virtual void   restoreState(const KConfigGroup &);

    virtual ConfigPageInfo  createConfigurationPage();
//     virtual AboutPageInfo   createAboutPage();



// IRadioClient
RECEIVERS:
    bool noticePowerChanged(bool /*on*/)                               { return false; }
    bool noticeStationChanged (const RadioStation &, int /*idx*/)      { return false; }
    bool noticeStationsChanged(const StationList &/*sl*/)              { return false; }
    bool noticePresetFileChanged(const QString &/*f*/)                 { return false; }
    bool noticeRDSStateChanged      (bool  /*enabled*/)                { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)             { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/)             { return false; }
    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/) { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/) { return false; }


protected:

// IRecCfg

    bool   setEncoderBuffer     (size_t BufferSize, size_t BufferCount);
    bool   setSoundFormat       (const SoundFormat &sf);
    bool   setMP3Quality        (int q);
    bool   setOggQuality        (float q);
    bool   setRecordingDirectory(const QString &dir, const recordingTemplate_t &templ);
    bool   setOutputFormat      (RecordingConfig::OutputFormat of);
    bool   setPreRecording      (bool enable, int seconds);
    bool   setRecordingConfig   (const RecordingConfig &cfg);

    void                           getEncoderBuffer(size_t &BufferSize, size_t &BufferCount) const;
    const SoundFormat             &getSoundFormat () const;
    int                            getMP3Quality () const;
    float                          getOggQuality () const;
    void                           getRecordingDirectory(QString &dir, recordingTemplate_t &templ) const;
    RecordingConfig::OutputFormat  getOutputFormat() const;
    bool                           getPreRecording(int &seconds) const;
    const RecordingConfig         &getRecordingConfig() const;

// ISoundStreamClient

    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid);

    bool startPlayback(SoundStreamID id);
    bool stopPlayback(SoundStreamID id);

    bool startRecording(SoundStreamID id, const recordingTemplate_t &templ);
    bool startRecordingWithFormat(SoundStreamID id, const SoundFormat &sf, SoundFormat &real_format, const recordingTemplate_t &templ);
    bool noticeSoundStreamData(SoundStreamID id, const SoundFormat &sf, const char *data, size_t size, size_t &consumed_size, const SoundMetaData &md);
    bool stopRecording(SoundStreamID id);
    bool isRecordingRunning(SoundStreamID id, bool &b, SoundFormat &sf) const;

    bool getSoundStreamDescription(SoundStreamID id, QString &descr)           const;
    bool getSoundStreamRadioStation(SoundStreamID id, const RadioStation *&rs) const;

    bool noticeSoundStreamClosed(SoundStreamID id);
    bool noticeSoundStreamChanged(SoundStreamID id);

    bool enumerateSourceSoundStreams(QMap<QString, SoundStreamID> &list) const;

protected:

    INLINE_IMPL_DEF_noticeConnectedI(IRadioClient);
    INLINE_IMPL_DEF_noticeConnectedI(IErrorLogClient);
    INLINE_IMPL_DEF_noticeConnectedI(IRecCfg);

protected slots:

    bool event(QEvent *e);

protected:

    bool startEncoder(SoundStreamID ssid, const RecordingConfig &cfg);
    void stopEncoder(SoundStreamID ssid);

protected:

    RecordingConfig     m_config;
    QMap<SoundStreamID, FileRingBuffer*>     m_PreRecordingBuffers;

    QMap<SoundStreamID, RecordingEncoding*>  m_EncodingThreads;
    QMap<SoundStreamID, SoundStreamID>       m_RawStreams2EncodedStreams;
    QMap<SoundStreamID, SoundStreamID>       m_EncodedStreams2RawStreams;
};

/* PreRecording Notes:  listen for startplayback, stopplayback, closestream
   manage map streamid => buffer
   set each started stream into capture mode
   put data into ringbuffers
   on capture start, feed everything into the encoder buffer,
      if encoderbuffer < prerecbuffer =>
          put as much as possible into encoder
          put new audio data into ring buffer

*/

#endif
