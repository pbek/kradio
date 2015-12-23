/***************************************************************************
                          mprissupport.h  -  description
                             -------------------
    copyright            : (C) 2014 by Pino Toscano
    email                : toscano.pino@tiscali.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DBUSSUPPORT_H
#define DBUSSUPPORT_H

#include <QObject>
#include "radio_interfaces.h"
#include "soundstreamclient_interfaces.h"
#include "pluginbase.h"

class MPRISSupport : public QObject,
                     public PluginBase,
                     public IRadioClient,
                     public ISoundStreamClient
{
Q_OBJECT

public:
    MPRISSupport(const QString &instanceID, const QString &name);
    ~MPRISSupport();

    virtual bool connectI (Interface *);
    virtual bool disconnectI (Interface *);

    virtual QString pluginClassName() const { return "MPRISSupport"; }

    virtual void           startPlugin();

    // PluginBase

RECEIVERS:
    INLINE_IMPL_DEF_noticeConnectedI(IErrorLogClient);

public:
    virtual void   saveState    (      KConfigGroup &) const;
    virtual void   restoreState (const KConfigGroup &);

    virtual ConfigPageInfo  createConfigurationPage();


    // IRadioClient methods

RECEIVERS:
    INLINE_IMPL_DEF_noticeConnectedI(IRadioClient);
    bool noticePowerChanged(bool on);
    bool noticeStationChanged(const RadioStation &rs, int idx);
    bool noticeStationsChanged(const StationList &sl);
    bool noticePresetFileChanged(const QString &f);

    bool noticeRDSStateChanged(bool enabled);
    bool noticeRDSRadioTextChanged(const QString &s);
    bool noticeRDSStationNameChanged(const QString &s);

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID id);
    bool noticeCurrentSoundStreamSinkIDChanged(SoundStreamID id);


    // ISoundStreamClient methods

RECEIVERS:
    void noticeConnectedI(ISoundStreamServer *s, bool pointer_valid);
    bool noticePlaybackVolumeChanged(SoundStreamID id, float volume);
    bool noticeSoundStreamChanged(SoundStreamID id);



Q_SIGNALS:
    void powerChanged(bool on);
    void RDSStateChanged(bool enabled);
    void RDSRadioTextChanged(const QString &s);
    void RDSStationNameChanged(const QString &s);
    void volumeChanged(double vol);
    void currentStreamChanged();


public:
    void showAllWidgetPlugins();
    bool isPlaying() const;
    bool isPaused() const;
    double volume() const;
    void setVolume(double vol);
    QVariantMap mprisMetadata() const;
    void stop();
    void play();
    void pause();
    void playPause();
};



#endif
