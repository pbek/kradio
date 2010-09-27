/***************************************************************************
                          reccfg_interfaces.h  -  description
                             -------------------
    begin                : Sun May 01 2005
    copyright            : (C) 2005 by Martin Witte
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

#ifndef KRADIO_RECCFG_INTERFACES_H
#define KRADIO_RECCFG_INTERFACES_H

#include "interfaces.h"
#include "recording-config.h"

INTERFACE(IRecCfg, IRecCfgClient)
{
public:
    IF_CON_DESTRUCTOR(IRecCfg, -1)

RECEIVERS:
    IF_RECEIVER(   setEncoderBuffer     (size_t BufferSize, size_t BufferCount)                        )
    IF_RECEIVER(   setSoundFormat       (const SoundFormat &sf)                                  )
    IF_RECEIVER(   setMP3Quality        (int q)                                                  )
    IF_RECEIVER(   setOggQuality        (float q)                                                )
    IF_RECEIVER(   setRecordingDirectory(const QString &dir, const QString &filenameTemplate)    )
    IF_RECEIVER(   setOutputFormat      (RecordingConfig::OutputFormat of)                       )
    IF_RECEIVER(   setPreRecording      (bool enable, int seconds)                               )
    IF_RECEIVER(   setRecordingConfig   (const RecordingConfig &cfg)                             )

SENDERS:
    IF_SENDER  (   notifyEncoderBufferChanged (size_t BufferSize, size_t BufferCount)                            )
    IF_SENDER  (   notifySoundFormatChanged   (const SoundFormat &sf)                                      )
    IF_SENDER  (   notifyMP3QualityChanged    (int q)                                                      )
    IF_SENDER  (   notifyOggQualityChanged    (float q)                                                    )
    IF_SENDER  (   notifyRecordingDirectoryChanged(const QString &dir, const QString &filenameTemplate)    )
    IF_SENDER  (   notifyOutputFormatChanged      (RecordingConfig::OutputFormat of)                       )
    IF_SENDER  (   notifyPreRecordingChanged      (bool enable, int seconds)                               )
    IF_SENDER  (   notifyRecordingConfigChanged   (const RecordingConfig &cfg)                             )

ANSWERS:
    IF_ANSWER  (   void                           getEncoderBuffer(size_t &BufferSize, size_t &BufferCount) const    )
    IF_ANSWER  (   const SoundFormat             &getSoundFormat () const                                      )
    IF_ANSWER  (   int                            getMP3Quality () const                                       )
    IF_ANSWER  (   float                          getOggQuality () const                                       )
    IF_ANSWER  (   void                           getRecordingDirectory(QString &dir, QString &templ) const    )
    IF_ANSWER  (   RecordingConfig::OutputFormat  getOutputFormat() const                                      )
    IF_ANSWER  (   bool                           getPreRecording(int &seconds) const                          )
    IF_ANSWER  (   const RecordingConfig         &getRecordingConfig() const                                   )
};



INTERFACE(IRecCfgClient, IRecCfg)
{
public:
    IF_CON_DESTRUCTOR(IRecCfgClient, 1)

SENDERS:
    IF_SENDER  (   sendEncoderBuffer     (size_t BufferSize, size_t BufferCount)                        )
    IF_SENDER  (   sendSoundFormat       (const SoundFormat &sf)                                  )
    IF_SENDER  (   sendMP3Quality        (int q)                                                  )
    IF_SENDER  (   sendOggQuality        (float q)                                                )
    IF_SENDER  (   sendRecordingDirectory(const QString &dir, const QString &filenameTemplate)    )
    IF_SENDER  (   sendOutputFormat      (RecordingConfig::OutputFormat of)                       )
    IF_SENDER  (   sendPreRecording      (bool enable, int seconds)                               )
    IF_SENDER  (   sendRecordingConfig   (const RecordingConfig &cfg)                             )

RECEIVERS:
    IF_RECEIVER(   noticeEncoderBufferChanged     (size_t BufferSize, size_t BufferCount)                        )
    IF_RECEIVER(   noticeSoundFormatChanged       (const SoundFormat &sf)                                  )
    IF_RECEIVER(   noticeMP3QualityChanged        (int q)                                                  )
    IF_RECEIVER(   noticeOggQualityChanged        (float q)                                                )
    IF_RECEIVER(   noticeRecordingDirectoryChanged(const QString &dir, const QString &filenameTemplate)    )
    IF_RECEIVER(   noticeOutputFormatChanged      (RecordingConfig::OutputFormat of)                       )
    IF_RECEIVER(   noticePreRecordingChanged      (bool enable, int seconds)                               )
    IF_RECEIVER(   noticeRecordingConfigChanged   (const RecordingConfig &cfg)                             )

QUERIES:
    IF_QUERY   (   void                           queryEncoderBuffer(size_t &BufferSize, size_t &BufferCount)    )
    IF_QUERY   (   const SoundFormat             &querySoundFormat ()                                      )
    IF_QUERY   (   int                            queryMP3Quality ()                                       )
    IF_QUERY   (   float                          queryOggQuality ()                                       )
    IF_QUERY   (   void                           queryRecordingDirectory(QString &dir, QString &templ)    )
    IF_QUERY   (   RecordingConfig::OutputFormat  queryOutputFormat()                                      )
    IF_QUERY   (   bool                           queryPreRecording(int &seconds)                          )
    IF_QUERY   (   const RecordingConfig         &queryRecordingConfig()                                   )

RECEIVERS:
    virtual void noticeConnectedI    (cmplInterface *, bool /*pointer_valid*/);
    virtual void noticeDisconnectedI (cmplInterface *, bool /*pointer_valid*/);
};

#endif
