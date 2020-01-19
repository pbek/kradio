/***************************************************************************
                       oss-sound-configuration.h  -  description
                             -------------------
    begin                : Thu Sep 30 2004
    copyright            : (C) 2004 by Martin Witte
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

#ifndef KRADIO_STREAMING_CONFIGURATION_H
#define KRADIO_STREAMING_CONFIGURATION_H

#include "ui_streaming-configuration-ui.h"
#include "streaming.h"


#include <QAbstractItemModel>


#define RATE_48000_IDX      0
#define RATE_44100_IDX      1
#define RATE_32000_IDX      2
#define RATE_22050_IDX      3
#define RATE_11025_IDX      4

#define CHANNELS_STEREO_IDX 0
#define CHANNELS_MONO_IDX   1

#define SIGN_SIGNED_IDX     0
#define SIGN_UNSIGNED_IDX   1

#define BITS_16_IDX         0
#define BITS_8_IDX          1

#define ENDIAN_LITTLE_IDX   0
#define ENDIAN_BIG_IDX      1

#define FORMAT_RAW_IDX      0


class StreamingConfigurationModel : public QAbstractItemModel
{
Q_OBJECT
public:
    enum MoveDirection { MoveUp, MoveDown };
    enum { DeviceRole = Qt::UserRole + 100, SoundFormatRole, BufferSizeRole };

    StreamingConfigurationModel(QObject *parent = 0);
    ~StreamingConfigurationModel();

    virtual int           columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant      data       (const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual Qt::ItemFlags flags      (const QModelIndex &index) const override;
    virtual QModelIndex   parent     (const QModelIndex &index) const override;
    virtual int           rowCount   (const QModelIndex &parent = QModelIndex()) const override;
    virtual bool          setData    (const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)   override;
    virtual QVariant      headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual QModelIndex   index      (int row, int column, const QModelIndex &parent = QModelIndex())       const override;
    virtual bool          removeRows (int row, int count,  const QModelIndex &parent = QModelIndex())             override;

    void clear();
    QModelIndex addDevice(const QString &device, const SoundFormat &format, int bufferSize);
    void moveRow(int row, MoveDirection dir);
    void editIndex(const QModelIndex &index, const SoundFormat &format, int bufferSize);

private:
    struct Data
    {
        QString device;
        SoundFormat format;
        int bufferSize;
    };

    QVector<Data> m_data;
};


class StreamingConfiguration : public QWidget,
                               public Ui_StreamingConfigurationUI
{
Q_OBJECT
public :
    StreamingConfiguration (QWidget *parent, StreamingDevice *streamer);
    ~StreamingConfiguration ();

protected slots:

    void slotOK();
    void slotCancel();

    void slotUpdateConfig();



    void slotNewPlaybackChannel();
    void slotDeletePlaybackChannel();
    void slotUpPlaybackChannel();
    void slotDownPlaybackChannel();

    void slotNewCaptureChannel();
    void slotDeleteCaptureChannel();
    void slotUpCaptureChannel();
    void slotDownCaptureChannel();

    void slotPlaybackSelectionChanged();
    void slotCaptureSelectionChanged();

    void slotUpdateSoundFormat();
    void slotSetDirty();

    void slotTabChanged(int);

protected:

    void setStreamOptions(const SoundFormat &sf, int BufferSize);
    void getStreamOptions(SoundFormat &sf, int &BufferSize) const ;


    StreamingConfigurationModel *m_PlaybackModel;
    StreamingConfigurationModel *m_CaptureModel;

    bool                    m_ignore_updates;
    bool                    m_dirty;
    StreamingDevice        *m_StreamingDevice;

};

#endif
