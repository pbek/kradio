/***************************************************************************
                          recording.h  -  description
                             -------------------
    begin                : Mi Aug 27 2003
    copyright            : (C) 2003 by Martin Witte
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

#ifndef KRADIO_RECORDING_H
#define KRADIO_RECORDING_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <sndfile.h>
/*#ifdef HAVE_LAME_LAME_H
    #include <lame/lame.h>
#endif
*/

#include <qobject.h>
#include <qstring.h>
#include <qmutex.h>
#include <qsemaphore.h>
#include <qthread.h>
#include <qtimer.h>
#include <qmap.h>

#include "../../src/libkradio/plugins.h"
#include "../../src/interfaces/timecontrol_interfaces.h"
#include "../../src/interfaces/soundstreamclient_interfaces.h"

#include "recording-config.h"
#include "reccfg_interfaces.h"
#include "encoder.h"
//#include <kradio/interfaces/radio_interfaces.h>

//#include "recording-context.h"

class RadioStation;
class StationList;
class QSocketNotifier;
class RecordingEncoding;

class Recording : public QObject,
                  public PluginBase,
                  public ISoundStreamClient,
                  public IRecCfg
{
Q_OBJECT
public:
    Recording(const QString &name);
    ~Recording();

    virtual QString pluginClassName() const { return "Recording"; }

    virtual const QString &name() const { return PluginBase::name(); }
    virtual       QString &name()       { return PluginBase::name(); }

    virtual bool   connectI(Interface *i);
    virtual bool   disconnectI(Interface *i);


    bool isRecording () const;


    // PluginBase

public:
    virtual void   saveState (KConfig *) const;
    virtual void   restoreState (KConfig *);

    virtual ConfigPageInfo  createConfigurationPage();
    virtual AboutPageInfo   createAboutPage();

protected:

// IRecCfg

    bool   setEncoderBuffer     (size_t BufferSize, size_t BufferCount);
    bool   setSoundFormat       (const SoundFormat &sf);
    bool   setMP3Quality        (int q);
    bool   setOggQuality        (float q);
    bool   setRecordingDirectory(const QString &dir);
    bool   setOutputFormat      (RecordingConfig::OutputFormat of);
    bool   setRecordingConfig   (const RecordingConfig &cfg);

    void                           getEncoderBuffer(size_t &BufferSize, size_t &BufferCount) const;
    const SoundFormat             &getSoundFormat () const;
    int                            getMP3Quality () const;
    float                          getOggQuality () const;
    const QString                 &getRecordingDirectory() const;
    RecordingConfig::OutputFormat  getOutputFormat() const;
    const RecordingConfig         &getRecordingConfig() const;

// ISoundStreamClient

    void noticeConnectedI (ISoundStreamServer *s, bool pointer_valid);

    bool startRecording(SoundStreamID id);
    bool startRecordingWithFormat(SoundStreamID id, const SoundFormat &sf, SoundFormat &real_format);
    bool noticeSoundStreamData(SoundStreamID id, const SoundFormat &sf, const char *data, size_t size, const SoundMetaData &md);
    bool stopRecording(SoundStreamID id);
    bool isRecordingRunning(SoundStreamID id, bool &b, SoundFormat &sf) const;

    bool getSoundStreamDescription(SoundStreamID id, QString &descr)           const;
    bool getSoundStreamRadioStation(SoundStreamID id, const RadioStation *&rs) const;

    bool noticeSoundStreamClosed(SoundStreamID id);
    bool noticeSoundStreamChanged(SoundStreamID id);

    bool enumerateSoundStreams(QMap<QString, SoundStreamID> &list) const;

protected slots:

    bool event(QEvent *e);

protected:

    bool startEncoder(SoundStreamID ssid, const RecordingConfig &cfg);
    void stopEncoder(SoundStreamID ssid);

protected:

    RecordingConfig     m_config;

    QMap<SoundStreamID, RecordingEncoding*>  m_EncodingThreads;
    QMap<SoundStreamID, SoundStreamID>       m_RawStreams2EncodedStreams;
    QMap<SoundStreamID, SoundStreamID>       m_EncodedStreams2RawStreams;
};


#endif
