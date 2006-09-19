/***************************************************************************
                       streaming-configuration.cpp  -  description
                             -------------------
    begin                : Thu Sep 30 2004
    copyright            : (C) 2004 by Martin Witte
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

#include <qcheckbox.h>

#include <kurlrequester.h>
#include <knuminput.h>
#include <klistview.h>
#include <kcombobox.h>
#include <knuminput.h>

#include "streaming-configuration.h"
#include "streaming.h"

StreamingConfiguration::StreamingConfiguration (QWidget *parent, StreamingDevice *streamer)
 : StreamingConfigurationUI(parent),
   m_ignore_updates(false),
   m_dirty(true),
   m_StreamingDevice(streamer)
{
    connect(m_pbNewPlaybackURL,    SIGNAL(clicked()), this, SLOT(slotNewPlaybackChannel()));
    connect(m_pbDeletePlaybackURL, SIGNAL(clicked()), this, SLOT(slotDeletePlaybackChannel()));
    connect(m_pbUpPlaybackURL,     SIGNAL(clicked()), this, SLOT(slotUpPlaybackChannel()));
    connect(m_pbDownPlaybackURL,   SIGNAL(clicked()), this, SLOT(slotDownPlaybackChannel()));
    connect(m_ListPlaybackURLs,    SIGNAL(selectionChanged()),           this, SLOT(slotPlaybackSelectionChanged()));
    connect(m_ListPlaybackURLs,    SIGNAL(itemRenamed(QListViewItem *)), this, SLOT(slotSetDirty()));

    connect(m_pbNewCaptureURL,     SIGNAL(clicked()), this, SLOT(slotNewCaptureChannel()));
    connect(m_pbDeleteCaptureURL,  SIGNAL(clicked()), this, SLOT(slotDeleteCaptureChannel()));
    connect(m_pbUpCaptureURL,      SIGNAL(clicked()), this, SLOT(slotUpCaptureChannel()));
    connect(m_pbDownCaptureURL,    SIGNAL(clicked()), this, SLOT(slotDownCaptureChannel()));
    connect(m_ListCaptureURLs,     SIGNAL(selectionChanged()),           this, SLOT(slotCaptureSelectionChanged()));
    connect(m_ListCaptureURLs,     SIGNAL(itemRenamed(QListViewItem *)), this, SLOT(slotSetDirty()));

    connect(m_cbBits,       SIGNAL(activated(int)),    this, SLOT(slotUpdateSoundFormat()));
    connect(m_cbChannels,   SIGNAL(activated(int)),    this, SLOT(slotUpdateSoundFormat()));
    connect(m_cbEndianess,  SIGNAL(activated(int)),    this, SLOT(slotUpdateSoundFormat()));
    connect(m_cbFormat,     SIGNAL(activated(int)),    this, SLOT(slotUpdateSoundFormat()));
    connect(m_cbRate,       SIGNAL(activated(int)),    this, SLOT(slotUpdateSoundFormat()));
    connect(m_cbSign,       SIGNAL(activated(int)),    this, SLOT(slotUpdateSoundFormat()));
    connect(m_sbBufferSize, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateSoundFormat()));

    m_ListPlaybackURLs->setAllColumnsShowFocus(true);
    m_ListPlaybackURLs->setSorting(-1);
    m_ListCaptureURLs->setAllColumnsShowFocus(true);
    m_ListCaptureURLs->setSorting(-1);

    slotCancel();
}


StreamingConfiguration::~StreamingConfiguration ()
{
}


void StreamingConfiguration::slotOK()
{
    if (!m_dirty)
        return;

    m_StreamingDevice->resetPlaybackStreams();
    m_StreamingDevice->resetCaptureStreams();

    QListViewItem *item = m_ListPlaybackURLs->firstChild();
    for (int i = 0; item; ++i, item = item->nextSibling()) {
        m_StreamingDevice->addPlaybackStream(item->text(1), m_PlaybackSoundFormats[i], m_PlaybackBufferSizes[i]);
    }

    item = m_ListCaptureURLs->firstChild();
    for (int i = 0; item; ++i, item = item->nextSibling()) {
        m_StreamingDevice->addCaptureStream(item->text(1), m_CaptureSoundFormats[i], m_CaptureBufferSizes[i]);
    }

    m_dirty = false;
}


void StreamingConfiguration::slotCancel()
{
    if (!m_dirty)
        return;

    const QStringList &playbackChannels = m_StreamingDevice->getPlaybackChannels();
    const QStringList &captureChannels  = m_StreamingDevice->getCaptureChannels();

    m_ListPlaybackURLs->clear();
    m_PlaybackBufferSizes.clear();
    m_PlaybackSoundFormats.clear();

    for (unsigned int i = 0; i < playbackChannels.size(); ++i) {
        SoundFormat sf;
        size_t      buffer_size;
        QString     url;
        m_StreamingDevice->getPlaybackStreamOptions(playbackChannels[i], url, sf, buffer_size);
        m_PlaybackSoundFormats.append(sf);
        m_PlaybackBufferSizes.append(buffer_size);

        QListViewItem *item = new QListViewItem(m_ListPlaybackURLs, m_ListPlaybackURLs->lastChild());
        item->setText(0, QString::number(m_ListPlaybackURLs->childCount()));
        item->setText(1, url);
        item->setRenameEnabled(1, true);
    }

    m_ListCaptureURLs->clear();
    m_CaptureBufferSizes.clear();
    m_CaptureSoundFormats.clear();

    for (unsigned int i = 0; i < captureChannels.size(); ++i) {
        SoundFormat sf;
        size_t      buffer_size;
        QString     url;
        m_StreamingDevice->getCaptureStreamOptions(captureChannels[i], url, sf, buffer_size);
        m_CaptureSoundFormats.append(sf);
        m_CaptureBufferSizes.append(buffer_size);

        QListViewItem *item = new QListViewItem(m_ListCaptureURLs, m_ListCaptureURLs->lastChild());
        item->setText(0, QString::number(m_ListCaptureURLs->childCount()));
        item->setText(1, url);
        item->setRenameEnabled(1, true);
    }
    slotPlaybackSelectionChanged();
    slotCaptureSelectionChanged();

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
    QListViewItem *item = new QListViewItem(m_ListPlaybackURLs, m_ListPlaybackURLs->lastChild());
    item->setText(0, QString::number(m_ListPlaybackURLs->childCount()));
    item->setText(1, "new channel");
    item->setRenameEnabled(1,true);
    item->startRename(1);

    m_PlaybackSoundFormats.append(SoundFormat());
    m_PlaybackBufferSizes.append(16*1024);
    int n = m_PlaybackSoundFormats.size();
    setStreamOptions(m_PlaybackSoundFormats[n-1], m_PlaybackBufferSizes[n-1]);
}


void StreamingConfiguration::slotDeletePlaybackChannel()
{
    slotSetDirty();
    QListViewItem *item = m_ListPlaybackURLs->selectedItem();
    if (item) {
        int idx = 0;
        QListViewItem *i    = m_ListPlaybackURLs->firstChild(),
                      *prev = NULL,
                      *next = item->nextSibling();
        for (; i && i != item; i = i->nextSibling()) {
            prev = i;
            ++idx;
        }
        if(next) {
            m_ListPlaybackURLs->setSelected(next, true);
        } else if (prev){
            m_ListPlaybackURLs->setSelected(prev, true);
        }
        int x = item->text(0).toUInt();
        for (i = next; i; i = i->nextSibling(), ++x) {
            i->setText(0, QString::number(x));
        }
        m_ListPlaybackURLs->takeItem(item);
        delete item;

        int n = m_PlaybackSoundFormats.size();
        m_PlaybackSoundFormats.remove(m_PlaybackSoundFormats.at(idx));
        m_PlaybackBufferSizes .remove(m_PlaybackBufferSizes.at(idx));
        idx = idx < n - 1 ? idx : n - 1; 
        setStreamOptions( m_PlaybackSoundFormats[idx], m_PlaybackBufferSizes[idx]);
        slotPlaybackSelectionChanged();
    }
}


void StreamingConfiguration::slotUpPlaybackChannel()
{
    slotSetDirty();
    QListViewItem *prev = NULL;
    QListViewItem *i    = m_ListPlaybackURLs->firstChild();
    QListViewItem *item = m_ListPlaybackURLs->selectedItem();
    int idx = 0;
    for (; i && i != item; i = i->nextSibling(), ++idx) {
        prev = i;
    }
    if (prev && item) {
        QString s = prev->text(1);
        prev->setText(1, item->text(1));
        item->setText(1, s);
        SoundFormat sf = m_PlaybackSoundFormats[idx];
        m_PlaybackSoundFormats[idx] = m_PlaybackSoundFormats[idx-1];
        m_PlaybackSoundFormats[idx-1] = sf;
        size_t size = m_PlaybackBufferSizes[idx];
        m_PlaybackBufferSizes[idx] = m_PlaybackBufferSizes[idx-1];
        m_PlaybackBufferSizes[idx-1] = size;
        m_ListPlaybackURLs->setSelected(prev, true);
    }
    m_ListPlaybackURLs->ensureItemVisible(prev);
}


void StreamingConfiguration::slotDownPlaybackChannel()
{
    slotSetDirty();
    QListViewItem *item = m_ListPlaybackURLs->selectedItem();
    QListViewItem *next = item ? item->nextSibling() : NULL;
    QListViewItem *i    = m_ListPlaybackURLs->firstChild();
    int idx = 0;
    for (; i && i != item; i = i->nextSibling()) {
        ++idx;
    }
    if (next && item) {
        QString s = next->text(1);
        next->setText(1, item->text(1));
        item->setText(1, s);
        SoundFormat sf = m_PlaybackSoundFormats[idx];
        m_PlaybackSoundFormats[idx] = m_PlaybackSoundFormats[idx+1];
        m_PlaybackSoundFormats[idx+1] = sf;
        size_t size = m_PlaybackBufferSizes[idx];
        m_PlaybackBufferSizes[idx] = m_PlaybackBufferSizes[idx+1];
        m_PlaybackBufferSizes[idx+1] = size;
        m_ListPlaybackURLs->setSelected(next, true);
    }
    m_ListPlaybackURLs->ensureItemVisible(next);
}



void StreamingConfiguration::slotNewCaptureChannel()
{
    slotSetDirty();
    QListViewItem *item = new QListViewItem(m_ListCaptureURLs, m_ListCaptureURLs->lastChild());
    item->setText(0, QString::number(m_ListCaptureURLs->childCount()));
    item->setText(1, "new channel");
    item->setRenameEnabled(1,true);
    item->startRename(1);

    m_CaptureSoundFormats.append(SoundFormat());
    m_CaptureBufferSizes.append(16*1024);
    int n = m_CaptureSoundFormats.size();
    setStreamOptions(m_CaptureSoundFormats[n-1], m_CaptureBufferSizes[n-1]);
}


void StreamingConfiguration::slotDeleteCaptureChannel()
{
    slotSetDirty();
    QListViewItem *item = m_ListCaptureURLs->selectedItem();
    if (item) {
        int idx = 0;
        QListViewItem *i    = m_ListCaptureURLs->firstChild(),
                      *prev = NULL,
                      *next = item->nextSibling();
        for (; i && i != item; i = i->nextSibling()) {
            prev = i;
            ++idx;
        }
        if (next) {
            m_ListCaptureURLs->setSelected(next, true);
        } else if (prev){
            m_ListCaptureURLs->setSelected(prev, true);
        }
        int x = item->text(0).toUInt();
        for (i = next; i; i = i->nextSibling(), ++x) {
            i->setText(0, QString::number(x));
        }
        m_ListCaptureURLs->takeItem(item);
        delete item;

        int n = m_CaptureSoundFormats.size();
        m_CaptureSoundFormats.remove(m_CaptureSoundFormats.at(idx));
        m_CaptureBufferSizes .remove(m_CaptureBufferSizes.at(idx));
        idx = idx < n - 1 ? idx : n - 1; 
        setStreamOptions( m_CaptureSoundFormats[idx], m_CaptureBufferSizes[idx]);
        slotCaptureSelectionChanged();
    }
}


void StreamingConfiguration::slotUpCaptureChannel()
{
    slotSetDirty();
    QListViewItem *prev = NULL;
    QListViewItem *i    = m_ListCaptureURLs->firstChild();
    QListViewItem *item = m_ListCaptureURLs->selectedItem();
    int idx = 0;
    for (; i && i != item; i = i->nextSibling(), ++idx) {
        prev = i;
    }
    if (prev && item) {
        QString s = prev->text(1);
        prev->setText(1, item->text(1));
        item->setText(1, s);
        SoundFormat sf = m_CaptureSoundFormats[idx];
        m_CaptureSoundFormats[idx] = m_CaptureSoundFormats[idx-1];
        m_CaptureSoundFormats[idx-1] = sf;
        size_t size = m_CaptureBufferSizes[idx];
        m_CaptureBufferSizes[idx] = m_CaptureBufferSizes[idx-1];
        m_CaptureBufferSizes[idx-1] = size;
        m_ListCaptureURLs->setSelected(prev, true);
    }
    m_ListCaptureURLs->ensureItemVisible(prev);
}


void StreamingConfiguration::slotDownCaptureChannel()
{
    slotSetDirty();
    QListViewItem *item = m_ListCaptureURLs->selectedItem();
    QListViewItem *next = item ? item->nextSibling() : NULL;
    QListViewItem *i    = m_ListCaptureURLs->firstChild();
    int idx = 0;
    for (; i && i != item; i = i->nextSibling()) {
        ++idx;
    }
    if (next && item) {
        QString s = next->text(1);
        next->setText(1, item->text(1));
        item->setText(1, s);
        SoundFormat sf = m_CaptureSoundFormats[idx];
        m_CaptureSoundFormats[idx] = m_CaptureSoundFormats[idx+1];
        m_CaptureSoundFormats[idx+1] = sf;
        size_t size = m_CaptureBufferSizes[idx];
        m_CaptureBufferSizes[idx] = m_CaptureBufferSizes[idx+1];
        m_CaptureBufferSizes[idx+1] = size;
        m_ListCaptureURLs->setSelected(next, true);
    }
    m_ListCaptureURLs->ensureItemVisible(next);
}





void StreamingConfiguration::slotPlaybackSelectionChanged()
{
    QListViewItem *item = m_ListPlaybackURLs->selectedItem();
    bool up_possible   = false;
    bool down_possible = false;
    if (item) {
        int idx = 0;
        QListViewItem *i = m_ListPlaybackURLs->firstChild();
        for (; i && i != item; i = i->nextSibling()) {
            ++idx;
        }
        up_possible   = idx > 0;
        down_possible = idx < m_ListPlaybackURLs->childCount() - 1;
        setStreamOptions(m_PlaybackSoundFormats[idx], m_PlaybackBufferSizes[idx]);

        item = m_ListCaptureURLs->selectedItem();
        if (item)
            m_ListCaptureURLs->setSelected(item, false);
    }
    QListViewItem *playback_item = m_ListPlaybackURLs->selectedItem();
    QListViewItem *capture_item  = m_ListCaptureURLs->selectedItem();
    bool e = (playback_item || capture_item);
    m_cbFormat    ->setEnabled(e);
    m_cbRate      ->setEnabled(e);
    m_cbBits      ->setEnabled(e);
    m_cbSign      ->setEnabled(e);
    m_cbChannels  ->setEnabled(e);
    m_cbEndianess ->setEnabled(e);
    m_sbBufferSize->setEnabled(e);
    m_pbUpPlaybackURL  ->setEnabled(up_possible);
    m_pbDownPlaybackURL->setEnabled(down_possible);
}


void StreamingConfiguration::slotCaptureSelectionChanged()
{
    QListViewItem *item = m_ListCaptureURLs->selectedItem();
    bool up_possible   = false;
    bool down_possible = false;
    if (item) {
        int idx = 0;
        QListViewItem *i = m_ListCaptureURLs->firstChild();
        for (; i && i != item; i = i->nextSibling()) {
            ++idx;
        }
        up_possible   = idx > 0;
        down_possible = idx < m_ListCaptureURLs->childCount() - 1;
        setStreamOptions(m_CaptureSoundFormats[idx], m_CaptureBufferSizes[idx]);

        item = m_ListPlaybackURLs->selectedItem();
        if (item)
            m_ListPlaybackURLs->setSelected(item, false);
    }
    QListViewItem *playback_item = m_ListPlaybackURLs->selectedItem();
    QListViewItem *capture_item  = m_ListCaptureURLs->selectedItem();
    bool e = (playback_item || capture_item);
    m_cbFormat    ->setEnabled(e);
    m_cbRate      ->setEnabled(e);
    m_cbBits      ->setEnabled(e);
    m_cbSign      ->setEnabled(e);
    m_cbChannels  ->setEnabled(e);
    m_cbEndianess ->setEnabled(e);
    m_sbBufferSize->setEnabled(e);
    m_pbUpCaptureURL  ->setEnabled(up_possible);
    m_pbDownCaptureURL->setEnabled(down_possible);
}

void StreamingConfiguration::slotSetDirty()
{
    m_dirty = true;
}

void StreamingConfiguration::slotUpdateSoundFormat()
{
    if (m_ignore_updates)
        return;

    slotSetDirty();
    QListViewItem *playback_item = m_ListPlaybackURLs->selectedItem();
    QListViewItem *capture_item  = m_ListCaptureURLs->selectedItem();
    if (playback_item) {
        int idx = 0;
        QListViewItem *i = m_ListPlaybackURLs->firstChild();
        for (; i && i != playback_item; i = i->nextSibling()) {
            ++idx;
        }
        getStreamOptions(m_PlaybackSoundFormats[idx], m_PlaybackBufferSizes[idx]);
    }
    else if (capture_item) {
        int idx = 0;
        QListViewItem *i = m_ListCaptureURLs->firstChild();
        for (; i && i != capture_item; i = i->nextSibling()) {
            ++idx;
        }
        getStreamOptions(m_CaptureSoundFormats[idx], m_CaptureBufferSizes[idx]);
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
    int idx_Endianess = ENDIAN_LITTLE_IDX;

    if (sf.m_Encoding == "raw") {
        idx_Format = FORMAT_RAW_IDX;
    }
    else {
        // ...
    }

    switch(sf.m_SampleRate) {
        case 44100 : idx_Rate = RATE_44100_IDX; break;
        case 22050 : idx_Rate = RATE_22050_IDX; break;
        case 11025 : idx_Rate = RATE_11025_IDX; break;
    }

    switch(sf.m_SampleBits) {
        case 8  : idx_Bits = BITS_8_IDX; break;
        case 16 : idx_Bits = BITS_16_IDX; break;
    }

    switch(sf.m_IsSigned) {
        case true  : idx_Sign = SIGN_SIGNED_IDX; break;
        case false : idx_Sign = SIGN_UNSIGNED_IDX; break;
    }

    switch(sf.m_Channels) {
        case 2: idx_Channels = CHANNELS_STEREO_IDX; break;
        case 1: idx_Channels = CHANNELS_MONO_IDX; break;
    }

    switch(sf.m_Endianess) {
        case LITTLE_ENDIAN: idx_Endianess = ENDIAN_LITTLE_IDX; break;
        case BIG_ENDIAN:    idx_Endianess = ENDIAN_BIG_IDX;    break;
    }

    m_cbFormat    ->setCurrentItem(idx_Format);
    m_cbRate      ->setCurrentItem(idx_Rate);
    m_cbBits      ->setCurrentItem(idx_Bits);
    m_cbSign      ->setCurrentItem(idx_Sign);
    m_cbChannels  ->setCurrentItem(idx_Channels);
    m_cbEndianess ->setCurrentItem(idx_Endianess);
    m_sbBufferSize->setValue(BufferSize / 1024);

    m_ignore_updates = false;
}


void StreamingConfiguration::getStreamOptions(SoundFormat &sf, int &BufferSize) const
{
    int idx_Format    = m_cbFormat    ->currentItem();
    int idx_Rate      = m_cbRate      ->currentItem();
    int idx_Bits      = m_cbBits      ->currentItem();
    int idx_Sign      = m_cbSign      ->currentItem();
    int idx_Channels  = m_cbChannels  ->currentItem();
    int idx_Endianess = m_cbEndianess ->currentItem();

    BufferSize = m_sbBufferSize->value() * 1024;

    if (idx_Format == FORMAT_RAW_IDX) {
        sf.m_Encoding = "raw";
    }
    else {
        // ...
    }

    switch(idx_Rate) {
        case RATE_44100_IDX : sf.m_SampleRate = 44100; break;
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

    switch(idx_Endianess) {
        case ENDIAN_LITTLE_IDX : sf.m_Endianess = LITTLE_ENDIAN; break;
        case ENDIAN_BIG_IDX    : sf.m_Endianess = BIG_ENDIAN;    break;
        default                : sf.m_Endianess = BYTE_ORDER;    break;
    }
}

#include "streaming-configuration.moc"
