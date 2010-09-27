/***************************************************************************
                          reccfg_interfaces.cpp  -  description
                             -------------------
    begin                : Sun May 01 2005
    copyright            : (C) 2005by Martin Witte
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

#include "reccfg_interfaces.h"

// IRecCfg

IF_IMPL_SENDER  (   IRecCfg::notifyEncoderBufferChanged (size_t BufferSize, size_t BufferCount),
                    noticeEncoderBufferChanged(BufferSize, BufferCount)
                );
IF_IMPL_SENDER  (   IRecCfg::notifySoundFormatChanged(const SoundFormat &sf),
                    noticeSoundFormatChanged(sf)
                );
IF_IMPL_SENDER  (   IRecCfg::notifyMP3QualityChanged(int q),
                    noticeMP3QualityChanged(q)
                );
IF_IMPL_SENDER  (   IRecCfg::notifyOggQualityChanged(float q),
                    noticeOggQualityChanged(q)
                );
IF_IMPL_SENDER  (   IRecCfg::notifyRecordingDirectoryChanged(const QString &dir, const QString &templ),
                    noticeRecordingDirectoryChanged(dir, templ)
                );
IF_IMPL_SENDER  (   IRecCfg::notifyOutputFormatChanged(RecordingConfig::OutputFormat of),
                    noticeOutputFormatChanged(of)
                );
IF_IMPL_SENDER  (   IRecCfg::notifyPreRecordingChanged(bool enable, int seconds),
                    noticePreRecordingChanged(enable, seconds)
                );
IF_IMPL_SENDER  (   IRecCfg::notifyRecordingConfigChanged   (const RecordingConfig &cfg),
                    noticeRecordingConfigChanged(cfg)
                );

// IRecCfgClient

IF_IMPL_SENDER  (   IRecCfgClient::sendEncoderBuffer (size_t BufferSize, size_t BufferCount),
                    setEncoderBuffer(BufferSize, BufferCount)
                );
IF_IMPL_SENDER  (   IRecCfgClient::sendSoundFormat(const SoundFormat &sf),
                    setSoundFormat(sf)
                );
IF_IMPL_SENDER  (   IRecCfgClient::sendMP3Quality(int q),
                    setMP3Quality(q)
                );
IF_IMPL_SENDER  (   IRecCfgClient::sendOggQuality(float q),
                    setOggQuality(q)
                );
IF_IMPL_SENDER  (   IRecCfgClient::sendRecordingDirectory(const QString &dir, const QString &templ),
                    setRecordingDirectory(dir, templ)
                );
IF_IMPL_SENDER  (   IRecCfgClient::sendOutputFormat(RecordingConfig::OutputFormat of),
                    setOutputFormat(of)
                );
IF_IMPL_SENDER  (   IRecCfgClient::sendPreRecording(bool enable, int seconds),
                    setPreRecording(enable, seconds)
                );
IF_IMPL_SENDER  (   IRecCfgClient::sendRecordingConfig(const RecordingConfig &cfg),
                    setRecordingConfig(cfg)
                );

IF_IMPL_QUERY   (   void IRecCfgClient::queryEncoderBuffer(size_t &BufferSize, size_t &BufferCount),
                    getEncoderBuffer(BufferSize, BufferCount),

                );

static SoundFormat defaultSoundFormat;
IF_IMPL_QUERY   (   const SoundFormat &IRecCfgClient::querySoundFormat (),
                    getSoundFormat(),
                    defaultSoundFormat
                );

IF_IMPL_QUERY   (   int IRecCfgClient::queryMP3Quality (),
                    getMP3Quality(),
                    7
                );

IF_IMPL_QUERY   (   float IRecCfgClient::queryOggQuality (),
                    getOggQuality(),
                    7
                );

static QString defaultRecDir("/tmp");
IF_IMPL_QUERY   (   void IRecCfgClient::queryRecordingDirectory(QString &dir, QString &templ),
                    getRecordingDirectory(dir, templ),

                );

IF_IMPL_QUERY   (   RecordingConfig::OutputFormat  IRecCfgClient::queryOutputFormat(),
                    getOutputFormat(),
                    RecordingConfig::outputWAV
                );

IF_IMPL_QUERY   (   bool IRecCfgClient::queryPreRecording(int &seconds),
                    getPreRecording(seconds),
                    false
                );

static RecordingConfig defaultRecConfig;
IF_IMPL_QUERY   (   const RecordingConfig &IRecCfgClient::queryRecordingConfig(),
                    getRecordingConfig(),
                    defaultRecConfig
                );

void IRecCfgClient::noticeConnectedI    (cmplInterface *, bool /*pointer_valid*/)
{
    size_t bs = 0, bc = 0;
    queryEncoderBuffer(bs, bc);
    noticeEncoderBufferChanged(bs, bc);
    noticeSoundFormatChanged(querySoundFormat());
    noticeMP3QualityChanged (queryMP3Quality());
    noticeOggQualityChanged (queryOggQuality());

    QString dir, templ;
    queryRecordingDirectory(dir, templ);
    noticeRecordingDirectoryChanged(dir, templ);

    noticeOutputFormatChanged(queryOutputFormat());
    int  s = 0;
    bool e = queryPreRecording(s);
    noticePreRecordingChanged(e, s);
    noticeRecordingConfigChanged(queryRecordingConfig());
}


void IRecCfgClient::noticeDisconnectedI (cmplInterface *, bool /*pointer_valid*/)
{
    size_t bs = 0, bc = 0;
    queryEncoderBuffer(bs, bc);
    noticeEncoderBufferChanged(bs, bc);
    noticeSoundFormatChanged(querySoundFormat());
    noticeMP3QualityChanged (queryMP3Quality());
    noticeOggQualityChanged (queryOggQuality());

    QString dir, templ;
    queryRecordingDirectory(dir, templ);
    noticeRecordingDirectoryChanged(dir, templ);

    noticeOutputFormatChanged(queryOutputFormat());
    int  s = 0;
    bool e = queryPreRecording(s);
    noticePreRecordingChanged(e, s);
    noticeRecordingConfigChanged(queryRecordingConfig());
}


