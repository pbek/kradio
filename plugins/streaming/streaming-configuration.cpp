/***************************************************************************
                       streaming-configuration.cpp  -  description
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

#include <QCheckBox>

#include <kurlrequester.h>
#include <knuminput.h>
#include <kcombobox.h>
#include <knuminput.h>

#include <klocalizedstring.h>

#include "streaming-configuration.h"
#include "streaming.h"

Q_DECLARE_METATYPE(SoundFormat)

StreamingConfigurationModel::StreamingConfigurationModel(QObject *parent)
 : QAbstractItemModel(parent)
{
}


StreamingConfigurationModel::~StreamingConfigurationModel()
{
}


int StreamingConfigurationModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 1;
}


QVariant StreamingConfigurationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_data.count() || index.column() != 0) {
        return QVariant();
    }

    const Data &d = m_data.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
    case DeviceRole:
        return d.device;
    case SoundFormatRole:
        return QVariant::fromValue(d.format);
    case BufferSizeRole:
        return d.bufferSize;
    }
    return QVariant();
}


Qt::ItemFlags StreamingConfigurationModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QAbstractItemModel::flags(index);
    if (index.isValid()) {
        f |= Qt::ItemIsEditable;
    }
    return f;
}


QVariant StreamingConfigurationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || section != 0 || role != Qt::DisplayRole) {
        return QVariant();
    }

    return i18n("URL");
}


QModelIndex StreamingConfigurationModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || row >= m_data.count() || column != 0 || parent.isValid()) {
        return QModelIndex();
    }

    return createIndex(row, column);
}


QModelIndex StreamingConfigurationModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}


bool StreamingConfigurationModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid() || row < 0 || count < 1) {
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row + count - 1);
    m_data.remove(row, count);
    endRemoveRows();
    return true;
}


int StreamingConfigurationModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_data.count();
}


bool StreamingConfigurationModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || !value.isValid()) {
        return false;
    }

    Data &d = m_data[index.row()];
    switch (role) {
    case Qt::EditRole:
        d.device = value.toString();
        break;
    default:
        return false;
    }
    emit dataChanged(index, index);
    return true;
}


void StreamingConfigurationModel::clear()
{
    beginResetModel();
    m_data.clear();
    endResetModel();
}


QModelIndex StreamingConfigurationModel::addDevice(const QString &device, const SoundFormat &format, int bufferSize)
{
    beginInsertRows(QModelIndex(), m_data.count(), m_data.count());
    Data d;
    d.device = device;
    d.format = format;
    d.bufferSize = bufferSize;
    m_data.append(d);
    endInsertRows();
    return createIndex(m_data.count() - 1, 0);
}


void StreamingConfigurationModel::moveRow(int row, MoveDirection dir)
{
    if (row < 0 || row >= m_data.count() || (dir == MoveUp && row == 0)
        || (dir == MoveDown && row == m_data.count() - 1)) {
        return;
    }

    if (dir == MoveUp) {
        --row;
    }
    beginMoveRows(QModelIndex(), row + 1, row + 1, QModelIndex(), row);
    const Data d = m_data.at(row + 1);
    m_data.remove(row + 1);
    m_data.insert(row, d);
    endMoveRows();
}


void StreamingConfigurationModel::editIndex(const QModelIndex &index, const SoundFormat &format, int bufferSize)
{
    if (!index.isValid()) {
        return;
    }

    Data &d = m_data[index.row()];
    d.format = format;
    d.bufferSize = bufferSize;
    emit dataChanged(index, index);
}


StreamingConfiguration::StreamingConfiguration (QWidget *parent, StreamingDevice *streamer)
 : QWidget(parent),
   m_ignore_updates(false),
   m_dirty(true),
   m_StreamingDevice(streamer)
{
    setupUi(this);

    m_pbNewCaptureURL    ->setIcon(KIcon("document-new"));
    m_pbNewPlaybackURL   ->setIcon(KIcon("document-new"));
    m_pbDeleteCaptureURL ->setIcon(KIcon("edit-delete"));
    m_pbDeletePlaybackURL->setIcon(KIcon("edit-delete"));
    m_pbUpCaptureURL     ->setIcon(KIcon("arrow-up"));
    m_pbUpPlaybackURL    ->setIcon(KIcon("arrow-up"));
    m_pbDownCaptureURL   ->setIcon(KIcon("arrow-down"));
    m_pbDownPlaybackURL  ->setIcon(KIcon("arrow-down"));

    m_PlaybackModel = new StreamingConfigurationModel(m_ListPlaybackURLs);
    m_ListPlaybackURLs->setModel(m_PlaybackModel);
    m_CaptureModel = new StreamingConfigurationModel(m_ListCaptureURLs);
    m_ListCaptureURLs->setModel(m_CaptureModel);

    connect(m_pbNewPlaybackURL,    SIGNAL(clicked()), this, SLOT(slotNewPlaybackChannel()));
    connect(m_pbDeletePlaybackURL, SIGNAL(clicked()), this, SLOT(slotDeletePlaybackChannel()));
    connect(m_pbUpPlaybackURL,     SIGNAL(clicked()), this, SLOT(slotUpPlaybackChannel()));
    connect(m_pbDownPlaybackURL,   SIGNAL(clicked()), this, SLOT(slotDownPlaybackChannel()));
    connect(m_ListPlaybackURLs->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotPlaybackSelectionChanged()));
    connect(m_PlaybackModel,       SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(slotSetDirty()));

    connect(m_pbNewCaptureURL,     SIGNAL(clicked()), this, SLOT(slotNewCaptureChannel()));
    connect(m_pbDeleteCaptureURL,  SIGNAL(clicked()), this, SLOT(slotDeleteCaptureChannel()));
    connect(m_pbUpCaptureURL,      SIGNAL(clicked()), this, SLOT(slotUpCaptureChannel()));
    connect(m_pbDownCaptureURL,    SIGNAL(clicked()), this, SLOT(slotDownCaptureChannel()));
    connect(m_ListCaptureURLs->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotCaptureSelectionChanged()));
    connect(m_CaptureModel,        SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(slotSetDirty()));

    connect(m_cbBits,       SIGNAL(activated(int)),    this, SLOT(slotUpdateSoundFormat()));
    connect(m_cbChannels,   SIGNAL(activated(int)),    this, SLOT(slotUpdateSoundFormat()));
    connect(m_cbEndianness, SIGNAL(activated(int)),    this, SLOT(slotUpdateSoundFormat()));
    connect(m_cbFormat,     SIGNAL(activated(int)),    this, SLOT(slotUpdateSoundFormat()));
    connect(m_cbRate,       SIGNAL(activated(int)),    this, SLOT(slotUpdateSoundFormat()));
    connect(m_cbSign,       SIGNAL(activated(int)),    this, SLOT(slotUpdateSoundFormat()));
    connect(m_sbBufferSize, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateSoundFormat()));

    connect(ktabwidget,     SIGNAL(currentChanged(int)), this, SLOT(slotTabChanged(int)));

    slotCancel();
}


StreamingConfiguration::~StreamingConfiguration ()
{
}


void StreamingConfiguration::slotOK()
{
    if (!m_dirty)
        return;

    m_StreamingDevice->resetPlaybackStreams(false);
    m_StreamingDevice->resetCaptureStreams(false);

    int rows = m_PlaybackModel->rowCount();
    for (int i = 0; i < rows; ++i) {
        const QModelIndex index = m_PlaybackModel->index(i, 0);
        const QString device = index.data(StreamingConfigurationModel::DeviceRole).toString();
        const SoundFormat sf = index.data(StreamingConfigurationModel::SoundFormatRole).value<SoundFormat>();
        const int buffer_size = index.data(StreamingConfigurationModel::BufferSizeRole).toInt();
        m_StreamingDevice->addPlaybackStream(device, sf, buffer_size, i == rows - 1);
    }

    rows = m_CaptureModel->rowCount();
    for (int i = 0; i < rows; ++i) {
        const QModelIndex index = m_CaptureModel->index(i, 0);
        const QString device = index.data(StreamingConfigurationModel::DeviceRole).toString();
        const SoundFormat sf = index.data(StreamingConfigurationModel::SoundFormatRole).value<SoundFormat>();
        const int buffer_size = index.data(StreamingConfigurationModel::BufferSizeRole).toInt();
        m_StreamingDevice->addCaptureStream(device, sf, buffer_size, i == rows - 1);
    }

    m_dirty = false;
}


void StreamingConfiguration::slotCancel()
{
    if (!m_dirty)
        return;

    const QStringList &playbackChannels = m_StreamingDevice->getPlaybackChannels();
    const QStringList &captureChannels  = m_StreamingDevice->getCaptureChannels();

    m_PlaybackModel->clear();

    for (int i = 0; i < playbackChannels.size(); ++i) {
        SoundFormat sf;
        size_t      buffer_size;
        KUrl        url;
        m_StreamingDevice->getPlaybackStreamOptions(playbackChannels[i], url, sf, buffer_size);
        m_PlaybackModel->addDevice(url.pathOrUrl(), sf, buffer_size);
    }

    m_CaptureModel->clear();

    for (int i = 0; i < captureChannels.size(); ++i) {
        SoundFormat sf;
        size_t      buffer_size;
        KUrl        url;
        m_StreamingDevice->getCaptureStreamOptions(captureChannels[i], url, sf, buffer_size);
        m_CaptureModel->addDevice(url.pathOrUrl(), sf, buffer_size);
    }
    slotTabChanged(ktabwidget->currentIndex());

    m_dirty = false;
}

void StreamingConfiguration::slotUpdateConfig()
{
    slotSetDirty();
    slotCancel();
}

void StreamingConfiguration::slotNewPlaybackChannel()
{
    slotSetDirty();

    const SoundFormat sf;
    const int buffer_size = 64*1024;
    setStreamOptions(sf, buffer_size);

    const QModelIndex index = m_PlaybackModel->addDevice(i18n("new channel"), sf, buffer_size);
    m_ListPlaybackURLs->setCurrentIndex(index);
    m_ListPlaybackURLs->edit(index);
}


void StreamingConfiguration::slotDeletePlaybackChannel()
{
    const QModelIndex index = m_ListPlaybackURLs->currentIndex();
    if (!index.isValid()) {
        return;
    }

    m_PlaybackModel->removeRow(index.row());
    slotSetDirty();
    slotPlaybackSelectionChanged();
}


void StreamingConfiguration::slotUpPlaybackChannel()
{
    const QModelIndex index = m_ListPlaybackURLs->currentIndex();
    if (!index.isValid()) {
        return;
    }

    m_PlaybackModel->moveRow(index.row(), StreamingConfigurationModel::MoveUp);
    slotSetDirty();
    slotPlaybackSelectionChanged();
}


void StreamingConfiguration::slotDownPlaybackChannel()
{
    const QModelIndex index = m_ListPlaybackURLs->currentIndex();
    if (!index.isValid()) {
        return;
    }

    m_PlaybackModel->moveRow(index.row(), StreamingConfigurationModel::MoveDown);
    slotSetDirty();
    slotPlaybackSelectionChanged();
}



void StreamingConfiguration::slotNewCaptureChannel()
{
    slotSetDirty();

    const SoundFormat sf;
    const int buffer_size = 64*1024;
    setStreamOptions(sf, buffer_size);

    const QModelIndex index = m_CaptureModel->addDevice(i18n("new channel"), sf, buffer_size);
    m_ListCaptureURLs->setCurrentIndex(index);
    m_ListCaptureURLs->edit(index);
}


void StreamingConfiguration::slotDeleteCaptureChannel()
{
    const QModelIndex index = m_ListCaptureURLs->currentIndex();
    if (!index.isValid()) {
        return;
    }

    m_CaptureModel->removeRow(index.row());
    slotSetDirty();
    slotCaptureSelectionChanged();
}


void StreamingConfiguration::slotUpCaptureChannel()
{
    const QModelIndex index = m_ListCaptureURLs->currentIndex();
    if (!index.isValid()) {
        return;
    }

    m_CaptureModel->moveRow(index.row(), StreamingConfigurationModel::MoveUp);
    slotSetDirty();
    slotCaptureSelectionChanged();
}


void StreamingConfiguration::slotDownCaptureChannel()
{
    const QModelIndex index = m_ListCaptureURLs->currentIndex();
    if (!index.isValid()) {
        return;
    }

    m_CaptureModel->moveRow(index.row(), StreamingConfigurationModel::MoveDown);
    slotSetDirty();
    slotCaptureSelectionChanged();
}





void StreamingConfiguration::slotPlaybackSelectionChanged()
{
    const QModelIndex index = m_ListPlaybackURLs->currentIndex();
    bool up_possible   = false;
    bool down_possible = false;
    bool e = false;
    if (index.isValid()) {
        int idx = index.row();
        up_possible   = idx > 0;
        down_possible = idx < m_PlaybackModel->rowCount() - 1;
        e = true;
        const SoundFormat sf = index.data(StreamingConfigurationModel::SoundFormatRole).value<SoundFormat>();
        const int buffer_size = index.data(StreamingConfigurationModel::BufferSizeRole).toInt();
        setStreamOptions(sf, buffer_size);
    }
    m_cbFormat    ->setEnabled(e);
    m_cbRate      ->setEnabled(e);
    m_cbBits      ->setEnabled(e);
    m_cbSign      ->setEnabled(e);
    m_cbChannels  ->setEnabled(e);
    m_cbEndianness ->setEnabled(e);
    m_sbBufferSize->setEnabled(e);
    m_pbUpPlaybackURL    ->setEnabled(up_possible);
    m_pbDownPlaybackURL  ->setEnabled(down_possible);
    m_pbDeletePlaybackURL->setEnabled(e);
}


void StreamingConfiguration::slotCaptureSelectionChanged()
{
    const QModelIndex index = m_ListCaptureURLs->currentIndex();
    bool up_possible   = false;
    bool down_possible = false;
    bool e = false;
    if (index.isValid()) {
        int idx = index.row();
        up_possible   = idx > 0;
        down_possible = idx < m_CaptureModel->rowCount() - 1;
        e = true;
        const SoundFormat sf = index.data(StreamingConfigurationModel::SoundFormatRole).value<SoundFormat>();
        const int buffer_size = index.data(StreamingConfigurationModel::BufferSizeRole).toInt();
        setStreamOptions(sf, buffer_size);
    }
    m_cbFormat    ->setEnabled(e);
    m_cbRate      ->setEnabled(e);
    m_cbBits      ->setEnabled(e);
    m_cbSign      ->setEnabled(e);
    m_cbChannels  ->setEnabled(e);
    m_cbEndianness ->setEnabled(e);
    m_sbBufferSize->setEnabled(e);
    m_pbUpCaptureURL    ->setEnabled(up_possible);
    m_pbDownCaptureURL  ->setEnabled(down_possible);
    m_pbDeleteCaptureURL->setEnabled(e);
}

void StreamingConfiguration::slotSetDirty()
{
    m_dirty = true;
}

void StreamingConfiguration::slotUpdateSoundFormat()
{
    if (m_ignore_updates)
        return;

    QTreeView *currentView;
    StreamingConfigurationModel *currentModel;
    if (ktabwidget->currentIndex() == 0) {
        currentView = m_ListCaptureURLs;
        currentModel = m_CaptureModel;
    } else {
        currentView = m_ListPlaybackURLs;
        currentModel = m_PlaybackModel;
    }
    const QModelIndex index = currentView->currentIndex();
    if (!index.isValid()) {
        return;
    }

    SoundFormat sf;
    int buffer_size;
    getStreamOptions(sf, buffer_size);
    m_ignore_updates = true;
    currentModel->editIndex(index, sf, buffer_size);
    m_ignore_updates = false;
    slotSetDirty();
}


void StreamingConfiguration::slotTabChanged(int index)
{
    if (index == 0) {
        slotCaptureSelectionChanged();
    } else {
        slotPlaybackSelectionChanged();
    }
}


void StreamingConfiguration::setStreamOptions(const SoundFormat &sf, int BufferSize)
{
    m_ignore_updates = true;

    int idx_Format    = FORMAT_RAW_IDX;
    int idx_Rate      = RATE_44100_IDX;
    int idx_Bits      = BITS_16_IDX;
    int idx_Sign      = SIGN_SIGNED_IDX;
    int idx_Channels  = CHANNELS_STEREO_IDX;
    int idx_Endianness = ENDIAN_LITTLE_IDX;

    if (sf.m_Encoding == "raw") {
        idx_Format = FORMAT_RAW_IDX;
    }
    else {
        // ...
    }

    switch(sf.m_SampleRate) {
        case 48000 : idx_Rate = RATE_48000_IDX; break;
        case 44100 : idx_Rate = RATE_44100_IDX; break;
        case 32000 : idx_Rate = RATE_32000_IDX; break;
        case 22050 : idx_Rate = RATE_22050_IDX; break;
        case 11025 : idx_Rate = RATE_11025_IDX; break;
    }

    switch(sf.m_SampleBits) {
        case 8  : idx_Bits = BITS_8_IDX; break;
        case 16 : idx_Bits = BITS_16_IDX; break;
    }

    idx_Sign = sf.m_IsSigned ? SIGN_SIGNED_IDX : SIGN_UNSIGNED_IDX;

    switch(sf.m_Channels) {
        case 2: idx_Channels = CHANNELS_STEREO_IDX; break;
        case 1: idx_Channels = CHANNELS_MONO_IDX; break;
    }

    switch(sf.m_Endianness) {
        case LITTLE_ENDIAN: idx_Endianness = ENDIAN_LITTLE_IDX; break;
        case BIG_ENDIAN:    idx_Endianness = ENDIAN_BIG_IDX;    break;
    }

    m_cbFormat    ->setCurrentIndex(idx_Format);
    m_cbRate      ->setCurrentIndex(idx_Rate);
    m_cbBits      ->setCurrentIndex(idx_Bits);
    m_cbSign      ->setCurrentIndex(idx_Sign);
    m_cbChannels  ->setCurrentIndex(idx_Channels);
    m_cbEndianness ->setCurrentIndex(idx_Endianness);
    m_sbBufferSize->setValue(BufferSize / 1024);

    m_ignore_updates = false;
}


void StreamingConfiguration::getStreamOptions(SoundFormat &sf, int &BufferSize) const
{
    int idx_Format    = m_cbFormat    ->currentIndex();
    int idx_Rate      = m_cbRate      ->currentIndex();
    int idx_Bits      = m_cbBits      ->currentIndex();
    int idx_Sign      = m_cbSign      ->currentIndex();
    int idx_Channels  = m_cbChannels  ->currentIndex();
    int idx_Endianness = m_cbEndianness ->currentIndex();

    BufferSize = m_sbBufferSize->value() * 1024;

    if (idx_Format == FORMAT_RAW_IDX) {
        sf.m_Encoding = "raw";
    }
    else {
        // ...
    }

    switch(idx_Rate) {
        case RATE_48000_IDX : sf.m_SampleRate = 48000; break;
        case RATE_44100_IDX : sf.m_SampleRate = 44100; break;
        case RATE_32000_IDX : sf.m_SampleRate = 32000; break;
        case RATE_22050_IDX : sf.m_SampleRate = 22050; break;
        case RATE_11025_IDX : sf.m_SampleRate = 11025; break;
        default             : sf.m_SampleRate = 44100; break;
    }

    switch(idx_Bits) {
        case BITS_8_IDX  : sf.m_SampleBits = 8;  break;
        case BITS_16_IDX : sf.m_SampleBits = 16; break;
        default          : sf.m_SampleBits = 16; break;
    }

    switch(idx_Sign) {
        case SIGN_SIGNED_IDX   : sf.m_IsSigned = true;  break;
        case SIGN_UNSIGNED_IDX : sf.m_IsSigned = false; break;
        default                : sf.m_IsSigned = true;  break;
    }

    switch(idx_Channels) {
        case CHANNELS_STEREO_IDX : sf.m_Channels = 2; break;
        case CHANNELS_MONO_IDX   : sf.m_Channels = 1; break;
        default                  : sf.m_Channels = 2; break;
    }

    switch(idx_Endianness) {
        case ENDIAN_LITTLE_IDX : sf.m_Endianness = LITTLE_ENDIAN; break;
        case ENDIAN_BIG_IDX    : sf.m_Endianness = BIG_ENDIAN;    break;
        default                : sf.m_Endianness = BYTE_ORDER;    break;
    }
}

#include "streaming-configuration.moc"
