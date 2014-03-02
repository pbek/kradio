/***************************************************************************
                          mprisplayer.cpp  -  description
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

#include "mprisplayer.h"
#include "mprissupport.h"

MPRISPlayer::MPRISPlayer(MPRISSupport *parent)
    : QDBusAbstractAdaptor(parent)
    , MPRISBase(this)
    , m_support(parent)
{
}

void MPRISPlayer::Next()
{
}

void MPRISPlayer::Previous()
{
}

void MPRISPlayer::Pause()
{
    m_support->pause();
}

void MPRISPlayer::PlayPause()
{
    m_support->playPause();
}

void MPRISPlayer::Stop()
{
    m_support->stop();
}

void MPRISPlayer::Play()
{
    m_support->play();
}

void MPRISPlayer::Seek(qlonglong Offset)
{
    Q_UNUSED(Offset)
}

void MPRISPlayer::SetPosition(const QDBusObjectPath& TrackId, qlonglong Position)
{
    Q_UNUSED(TrackId)
    Q_UNUSED(Position)
}

void MPRISPlayer::OpenUri(const QString &Uri)
{
    Q_UNUSED(Uri)
}

void MPRISPlayer::slotPowerChanged(bool on)
{
    Q_UNUSED(on)
    QVariantMap properties;
    properties["PlaybackStatus"] = PlaybackStatus();
    signalPropertiesChange(properties);
}

void MPRISPlayer::slotRDSStateChanged(bool enabled)
{
    Q_UNUSED(enabled)
    QVariantMap properties;
    properties["Metadata"] = Metadata();
    signalPropertiesChange(properties);
}

void MPRISPlayer::slotRDSRadioTextChanged(const QString &s)
{
    Q_UNUSED(s)
    QVariantMap properties;
    properties["Metadata"] = Metadata();
    signalPropertiesChange(properties);
}

void MPRISPlayer::slotRDSStationNameChanged(const QString &s)
{
    Q_UNUSED(s)
    QVariantMap properties;
    properties["Metadata"] = Metadata();
    signalPropertiesChange(properties);
}

void MPRISPlayer::slotVolumeChanged(double vol)
{
    Q_UNUSED(vol)
    QVariantMap properties;
    properties["Volume"] = Volume();
    signalPropertiesChange(properties);
}

void MPRISPlayer::slotCurrentStreamChanged()
{
    QVariantMap properties;
    properties["Metadata"] = Metadata();
    signalPropertiesChange(properties);
}

QString MPRISPlayer::PlaybackStatus() const
{
    if (m_support->isPlaying()) {
        return m_support->isPaused() ? "Paused" : "Playing";
    }
    return "Stopped";
}

QString MPRISPlayer::LoopStatus() const
{
    return "None";
}

void MPRISPlayer::setLoopStatus(const QString& loopStatus)
{
    Q_UNUSED(loopStatus)
}

double MPRISPlayer::Rate() const
{
    return 1.0;
}

void MPRISPlayer::setRate(double rate)
{
    Q_UNUSED(rate)
}

bool MPRISPlayer::Shuffle() const
{
    return false;
}

void MPRISPlayer::setShuffle(bool shuffle)
{
    Q_UNUSED(shuffle)
}

QVariantMap MPRISPlayer::Metadata() const
{
    return m_support->mprisMetadata();
}

double MPRISPlayer::Volume() const
{
    return m_support->volume();
}

void MPRISPlayer::setVolume(double volume)
{
    m_support->setVolume(volume);
}

qlonglong MPRISPlayer::Position() const
{
    return 0;
}

double MPRISPlayer::MinimumRate() const
{
    return 1.0;
}

double MPRISPlayer::MaximumRate() const
{
    return 1.0;
}

bool MPRISPlayer::CanGoNext() const
{
    return false;
}

bool MPRISPlayer::CanGoPrevious() const
{
    return false;
}

bool MPRISPlayer::CanPlay() const
{
    // FIXME
    return true;
}

bool MPRISPlayer::CanPause() const
{
    // FIXME
    return true;
}

bool MPRISPlayer::CanSeek() const
{
    return false;
}

bool MPRISPlayer::CanControl() const
{
    return true;
}


#include <mprisplayer.moc>
