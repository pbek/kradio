/***************************************************************************
                          mprisplayer.h  -  description
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

#ifndef MPRISPLAYER_H
#define MPRISPLAYER_H

#include "mprisbase.h"

#include <QObject>
#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

class MPRISSupport;

class MPRISPlayer : public QDBusAbstractAdaptor,
                    public MPRISBase
{
Q_OBJECT
Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")

Q_PROPERTY(QString PlaybackStatus READ PlaybackStatus)
Q_PROPERTY(QString LoopStatus READ LoopStatus WRITE setLoopStatus)
Q_PROPERTY(double Rate READ Rate WRITE setRate)
Q_PROPERTY(bool Shuffle READ Shuffle WRITE setShuffle)
Q_PROPERTY(QVariantMap Metadata READ Metadata)
Q_PROPERTY(double Volume READ Volume WRITE setVolume)
Q_PROPERTY(qlonglong Position READ Position)
Q_PROPERTY(double MinimumRate READ MinimumRate)
Q_PROPERTY(double MaximumRate READ MaximumRate)
Q_PROPERTY(bool CanGoNext READ CanGoNext)
Q_PROPERTY(bool CanGoPrevious READ CanGoPrevious)
Q_PROPERTY(bool CanPlay READ CanPlay)
Q_PROPERTY(bool CanPause READ CanPause)
Q_PROPERTY(bool CanSeek READ CanSeek)
Q_PROPERTY(bool CanControl READ CanControl)

public:
    MPRISPlayer(MPRISSupport *parent);

Q_SIGNALS:
    void Seeked(qlonglong Position);

public Q_SLOTS:
    void Next();
    void Previous();
    void Pause();
    void PlayPause();
    void Stop();
    void Play();
    void Seek(qlonglong Offset);
    void SetPosition(const QDBusObjectPath& TrackId, qlonglong Position);
    void OpenUri(const QString &Uri);

private Q_SLOTS:
    friend class MPRISSupport;
    void slotPowerChanged(bool on);
    void slotRDSStateChanged(bool enabled);
    void slotRDSRadioTextChanged(const QString &s);
    void slotRDSStationNameChanged(const QString &s);
    void slotVolumeChanged(double vol);
    void slotCurrentStreamChanged();

private:
    QString PlaybackStatus() const;
    QString LoopStatus() const;
    void setLoopStatus(const QString& loopStatus);
    double Rate() const;
    void setRate(double rate);
    bool Shuffle() const;
    void setShuffle(bool shuffle);
    QVariantMap Metadata() const;
    double Volume() const;
    void setVolume(double volume);
    qlonglong Position() const;
    double MinimumRate() const;
    double MaximumRate() const;
    bool CanGoNext() const;
    bool CanGoPrevious() const;
    bool CanPlay() const;
    bool CanPause() const;
    bool CanSeek() const;
    bool CanControl() const;

    MPRISSupport *m_support;
};

#endif
