/***************************************************************************
                          alsa-sounddevice-model.h  -  description
                             -------------------
    copyright            : (C) 2015 by Pino Toscano
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

#ifndef KRADIO_ALSA_SOUNDDEVICE_MODEL_H
#define KRADIO_ALSA_SOUNDDEVICE_MODEL_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QString>

#include "alsa-sound.h"

template <typename T>
class AlsaSoundBaseModel : public QAbstractListModel
{
public:
    AlsaSoundBaseModel(QObject *parent = 0) : QAbstractListModel(parent) {}

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        if (index.column() != 0 || index.row() < 0 || index.row() >= m_list.count())
            return QVariant();

        const T &item = m_list.at(index.row());
        switch(role) {
            case Qt::DisplayRole:
                return customDisplay(item);
            case Qt::UserRole:
                return customData(item);
        }
        return QVariant();
    }

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const
    {
        return parent.isValid() ? 0 : m_list.count();
    }

    bool reload()
    {
        const QList<T> newList = doReload();
        if (newList == m_list)
            return false;

        beginResetModel();
        m_list = newList;
        endResetModel();
        return true;
    }

protected:
    virtual QList<T> doReload() const = 0;
    virtual QString customDisplay(const T &item) const = 0;
    virtual QString customData(const T &item) const = 0;

private:
    QList<T> m_list;
};


class AlsaSoundBaseDeviceModel : public AlsaSoundBaseModel<AlsaSoundDeviceMetaData>
{
public:
    AlsaSoundBaseDeviceModel(QObject *parent = 0) : AlsaSoundBaseModel<AlsaSoundDeviceMetaData>(parent) {}

protected:
    virtual QString customDisplay(const AlsaSoundDeviceMetaData &item) const
    {
        QString res = item.cardDescription();
        if (res.length()) res += ", ";
        res += item.deviceVerboseDescription();
        return res;
    }

    virtual QString customData(const AlsaSoundDeviceMetaData &item) const
    {
        return item.pcmDeviceName();
    }
};


class AlsaSoundBaseMixerModel : public AlsaSoundBaseModel<AlsaMixerMetaData>
{
public:
    AlsaSoundBaseMixerModel(QObject *parent = 0) : AlsaSoundBaseModel<AlsaMixerMetaData>(parent) {}

protected:
    virtual QString customDisplay(const AlsaMixerMetaData &item) const
    {
        return item.cardDescription();
    }

    virtual QString customData(const AlsaMixerMetaData &item) const
    {
        return item.mixerCardName();
    }
};


class AlsaSoundPlaybackDeviceModel : public AlsaSoundBaseDeviceModel
{
public:
    AlsaSoundPlaybackDeviceModel(QObject *parent = 0) : AlsaSoundBaseDeviceModel(parent) {}

protected:
    virtual QList<AlsaSoundDeviceMetaData> doReload() const
    {
        return AlsaSoundDevice::getPCMPlaybackDeviceDescriptions();
    }
};


class AlsaSoundCaptureDeviceModel : public AlsaSoundBaseDeviceModel
{
public:
    AlsaSoundCaptureDeviceModel(QObject *parent = 0) : AlsaSoundBaseDeviceModel(parent) {}

protected:
    virtual QList<AlsaSoundDeviceMetaData> doReload() const
    {
        return AlsaSoundDevice::getPCMCaptureDeviceDescriptions();
    }
};


class AlsaSoundPlaybackMixerModel : public AlsaSoundBaseMixerModel
{
public:
    AlsaSoundPlaybackMixerModel(QObject *parent = 0) : AlsaSoundBaseMixerModel(parent) {}

protected:
    virtual QList<AlsaMixerMetaData> doReload() const
    {
        return AlsaSoundDevice::getPlaybackMixerDescriptions();
    }
};


class AlsaSoundCaptureMixerModel : public AlsaSoundBaseMixerModel
{
public:
    AlsaSoundCaptureMixerModel(QObject *parent = 0) : AlsaSoundBaseMixerModel(parent) {}

protected:
    virtual QList<AlsaMixerMetaData> doReload() const
    {
        return AlsaSoundDevice::getCaptureMixerDescriptions();
    }
};


#endif
