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

    virtual bool connectI    (Interface *) override;
    virtual bool disconnectI (Interface *) override;

    virtual QString pluginClassName() const override { return QString::fromLatin1("MPRISSupport"); }

    virtual void           startPlugin() override;

    // PluginBase

RECEIVERS:
    INLINE_IMPL_DEF_noticeConnectedI(IErrorLogClient);

public:
    virtual void   saveState    (      KConfigGroup &) const override;
    virtual void   restoreState (const KConfigGroup &)       override;


    // IRadioClient methods

RECEIVERS:
    INLINE_IMPL_DEF_noticeConnectedI(IRadioClient);
    bool noticePowerChanged(bool on) override;
    bool noticeStationChanged(const RadioStation &rs, int idx) override;
    bool noticeStationsChanged(const StationList &sl) override;
    bool noticePresetFileChanged(const QString &f) override;

    bool noticeRDSStateChanged(bool enabled)           override;
    bool noticeRDSRadioTextChanged(const QString &s)   override;
    bool noticeRDSStationNameChanged(const QString &s) override;

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID id) override;
    bool noticeCurrentSoundStreamSinkIDChanged(SoundStreamID id)   override;


    // ISoundStreamClient methods

RECEIVERS:
    void noticeConnectedI(ISoundStreamServer *s, bool pointer_valid) override;
    bool noticePlaybackVolumeChanged(SoundStreamID id, float volume) override;
    bool noticeSoundStreamChanged(SoundStreamID id) override;



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
