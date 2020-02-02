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

#include <QObject>
#include <QString>
#include <QMap>

#include "pluginbase.h"
#include "timecontrol_interfaces.h"
#include "soundstreamclient_interfaces.h"
#include "radio_interfaces.h"

#include "recording-config.h"
#include "reccfg_interfaces.h"
#include "encoder.h"

class RadioStation;
class StationList;
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

    virtual QString pluginClassName() const override { return QString::fromLatin1("Recording"); }

    virtual bool   connectI   (Interface *i) override;
    virtual bool   disconnectI(Interface *i) override;


    bool isRecording () const;


    // PluginBase

public:
    virtual void   saveState   (      KConfigGroup &) const override;
    virtual void   restoreState(const KConfigGroup &)       override;

    virtual ConfigPageInfo  createConfigurationPage() override;



// IRadioClient
RECEIVERS:
    bool noticePowerChanged(bool /*on*/)                               override { return false; }
    bool noticeStationChanged (const RadioStation &, int /*idx*/)      override { return false; }
    bool noticeStationsChanged(const StationList &/*sl*/)              override { return false; }
    bool noticePresetFileChanged(const QUrl &/*f*/)                    override { return false; }
    bool noticeRDSStateChanged      (bool  /*enabled*/)                override { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)             override { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/)             override { return false; }
    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/) override { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/) override { return false; }


protected:

// IRecCfg

    bool   setEncoderBuffer     (size_t BufferSize, size_t BufferCount) override;
    bool   setSoundFormat       (const SoundFormat &sf) override;
    bool   setMP3Quality        (int q)   override;
    bool   setOggQuality        (float q) override;
    bool   setRecordingDirectory(const QString &dir, const recordingTemplate_t &templ) override;
    bool   setOutputFormat      (RecordingConfig::OutputFormat of) override;
    bool   setPreRecording      (bool enable, int seconds)         override;
    bool   setRecordingConfig   (const RecordingConfig &cfg)       override;

    void                           getEncoderBuffer(size_t &BufferSize, size_t &BufferCount) const override;
    const SoundFormat             &getSoundFormat () const override;
    int                            getMP3Quality () const override;
    float                          getOggQuality () const override;
    void                           getRecordingDirectory(QString &dir, recordingTemplate_t &templ) const override;
    RecordingConfig::OutputFormat  getOutputFormat() const override;
    bool                           getPreRecording(int &seconds) const override;
    const RecordingConfig         &getRecordingConfig() const override;

// ISoundStreamClient

    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid) override;

    bool startPlayback(SoundStreamID id) override;
    bool stopPlayback(SoundStreamID id)  override;

    bool startRecording(SoundStreamID id, const recordingTemplate_t &templ) override;
    bool startRecordingWithFormat(SoundStreamID id, const SoundFormat &sf, SoundFormat &real_format, const recordingTemplate_t &templ) override;
    bool noticeSoundStreamData(SoundStreamID id, const SoundFormat &sf, const char *data, size_t size, size_t &consumed_size, const SoundMetaData &md) override;
    bool stopRecording(SoundStreamID id) override;
    bool isRecordingRunning(SoundStreamID id, bool &b, SoundFormat &sf) const override;

    bool getSoundStreamDescription(SoundStreamID id, QString &descr)           const override;
    bool getSoundStreamRadioStation(SoundStreamID id, const RadioStation *&rs) const override;

    bool noticeSoundStreamClosed(SoundStreamID id)  override;
    bool noticeSoundStreamChanged(SoundStreamID id) override;

    bool enumerateSourceSoundStreams(QMap<QString, SoundStreamID> &list) const override;

protected:

    INLINE_IMPL_DEF_noticeConnectedI(IRadioClient);
    INLINE_IMPL_DEF_noticeConnectedI(IErrorLogClient);
    INLINE_IMPL_DEF_noticeConnectedI(IRecCfg);

protected slots:

    bool event(QEvent *e) override;

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
