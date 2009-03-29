/***************************************************************************
                          recording-configuration.cpp  -  description
                             -------------------
    begin                : So Aug 31 2003
    copyright            : (C) 2003 by Martin Witte
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

#include "recording-configuration.h"
//#include "recording-context.h"

#include <QtGui/QSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>

#include <kurlrequester.h>
#include <kcombobox.h>
#include <ktabwidget.h>


RecordingConfiguration::RecordingConfiguration (QWidget *parent)
    : QWidget(parent),
      m_dirty(true),
      m_ignore_gui_updates(false)
{
    setupUi(this);

    editDirectory->setMode(KFile::Directory | KFile::ExistingOnly);

    QObject::connect(editFileFormat, SIGNAL(activated(int)),
                     this,           SLOT(slotFormatSelectionChanged()));
    QObject::connect(editBits,       SIGNAL(activated(int)),
                     this,           SLOT(slotFormatSelectionChanged()));

    connect(editRate,                     SIGNAL(activated(int)),               SLOT(slotSetDirty()));
    connect(editBits,                     SIGNAL(activated(int)),               SLOT(slotSetDirty()));
    connect(editSign,                     SIGNAL(activated(int)),               SLOT(slotSetDirty()));
    connect(editEndianess,                SIGNAL(activated(int)),               SLOT(slotSetDirty()));
    connect(editChannels,                 SIGNAL(activated(int)),               SLOT(slotSetDirty()));
    connect(editFileFormat,               SIGNAL(activated(int)),               SLOT(slotSetDirty()));
    connect(editMP3Quality,               SIGNAL(valueChanged(int)),            SLOT(slotSetDirty()));
    connect(editOggQuality,               SIGNAL(valueChanged(int)),            SLOT(slotSetDirty()));
    connect(editDirectory,                SIGNAL(textChanged(const QString &)), SLOT(slotSetDirty()));
    connect(editBufferSize,               SIGNAL(valueChanged(int)),            SLOT(slotSetDirty()));
    connect(editBufferCount,              SIGNAL(valueChanged(int)),            SLOT(slotSetDirty()));
    connect(m_spinboxPreRecordingSeconds, SIGNAL(valueChanged(int)),            SLOT(slotSetDirty()));
    connect(m_checkboxPreRecordingEnable, SIGNAL(toggled(bool)),                SLOT(slotSetDirty()));

// attention: remove items with higher index first ;-) otherwise indexes are not valid
#ifndef HAVE_OGG
    editFileFormat->removeItem(FORMAT_OGG_IDX_ORG);
    delete editOggQuality;
    editOggQuality = NULL;
    delete labelOggQuality;
    labelOggQuality = NULL;
#endif
#ifndef HAVE_LAME
    editFileFormat->removeItem(FORMAT_MP3_IDX_ORG);
    delete editMP3Quality;
    editMP3Quality = NULL;
    delete labelMP3Quality;
    labelMP3Quality = NULL;
#endif
}


RecordingConfiguration::~RecordingConfiguration ()
{
}


void RecordingConfiguration::setGUIBuffers(const RecordingConfig &c)
{
    editBufferSize->setValue(c.m_EncodeBufferSize / 1024);
    editBufferCount->setValue(c.m_EncodeBufferCount);
}

void RecordingConfiguration::setGUIDirectories(const RecordingConfig &c)
{
    editDirectory->setUrl(c.m_Directory);
}

void RecordingConfiguration::setGUISoundFormat(const RecordingConfig &c)
{
    switch (c.m_SoundFormat.m_SampleBits) {
        case 8 : editBits->setCurrentIndex(BITS_8_IDX ); break;
        case 16: editBits->setCurrentIndex(BITS_16_IDX); break;
        default: editBits->setCurrentIndex(BITS_16_IDX);
    }
    switch (c.m_SoundFormat.m_Channels) {
        case 1 : editChannels->setCurrentIndex(CHANNELS_MONO_IDX); break;
        case 2 : editChannels->setCurrentIndex(CHANNELS_STEREO_IDX); break;
        default: editChannels->setCurrentIndex(CHANNELS_STEREO_IDX); break;
    }
    switch (c.m_SoundFormat.m_IsSigned) {
        case 0 : editSign->setCurrentIndex(SIGN_UNSIGNED_IDX); break;
        case 1 : editSign->setCurrentIndex(SIGN_SIGNED_IDX); break;
        default: editSign->setCurrentIndex(SIGN_SIGNED_IDX); break;
    }
    switch (c.m_SoundFormat.m_SampleRate) {
        case 48000: editRate->setCurrentIndex(RATE_48000_IDX); break;
        case 44100: editRate->setCurrentIndex(RATE_44100_IDX); break;
        case 32000: editRate->setCurrentIndex(RATE_32000_IDX); break;
        case 22050: editRate->setCurrentIndex(RATE_22050_IDX); break;
        case 11025: editRate->setCurrentIndex(RATE_11025_IDX); break;
        default:    editRate->setCurrentIndex(RATE_44100_IDX); break;
    }
    switch (c.m_SoundFormat.m_Endianess) {
        case BIG_ENDIAN    : editEndianess->setCurrentIndex(ENDIAN_BIG_IDX); break;
        case LITTLE_ENDIAN : editEndianess->setCurrentIndex(ENDIAN_LITTLE_IDX); break;
        default:             editEndianess->setCurrentIndex(ENDIAN_LITTLE_IDX); break;
    }
}

void RecordingConfiguration::setGUIOutputFormat(const RecordingConfig &c)
{
    switch (c.m_OutputFormat) {
        case RecordingConfig::outputWAV:  editFileFormat->setCurrentIndex(FORMAT_WAV_IDX);  break;
        case RecordingConfig::outputAIFF: editFileFormat->setCurrentIndex(FORMAT_AIFF_IDX); break;
        case RecordingConfig::outputAU:   editFileFormat->setCurrentIndex(FORMAT_AU_IDX);   break;
        case RecordingConfig::outputRAW:  editFileFormat->setCurrentIndex(FORMAT_RAW_IDX);  break;
#ifdef HAVE_LAME
        case RecordingConfig::outputMP3:  editFileFormat->setCurrentIndex(FORMAT_MP3_IDX);  break;
#endif
#ifdef HAVE_OGG
        case RecordingConfig::outputOGG:  editFileFormat->setCurrentIndex(FORMAT_OGG_IDX);  break;
#endif
        default:                          editFileFormat->setCurrentIndex(FORMAT_WAV_IDX);  break;
    }
}

void RecordingConfiguration::setGUIEncoderQuality(const RecordingConfig &c)
{
#ifdef HAVE_LAME
    editMP3Quality->setValue(c.m_mp3Quality);
#endif
#ifdef HAVE_OGG
    editOggQuality->setValue((int)(c.m_oggQuality * 9));
#endif
}


void RecordingConfiguration::setGUIPreRecording(const RecordingConfig &c)
{
    m_spinboxPreRecordingSeconds->setValue(c.m_PreRecordingSeconds);
    m_checkboxPreRecordingEnable->setChecked(c.m_PreRecordingEnable);
}


void RecordingConfiguration::slotOK()
{
    if (m_dirty) {
        storeConfig();
        sendRecordingConfig(m_RecordingConfig);
        m_dirty = false;
    }
}


void RecordingConfiguration::storeConfig()
{
    RecordingConfig &c = m_RecordingConfig;

    c.m_EncodeBufferSize  = editBufferSize->value() * 1024;
    c.m_EncodeBufferCount = editBufferCount->value();

    c.m_Directory = editDirectory->url().pathOrUrl();

    switch(editRate->currentIndex()) {
        case RATE_48000_IDX: c.m_SoundFormat.m_SampleRate = 48000; break;
        case RATE_44100_IDX: c.m_SoundFormat.m_SampleRate = 44100; break;
        case RATE_32000_IDX: c.m_SoundFormat.m_SampleRate = 32000; break;
        case RATE_22050_IDX: c.m_SoundFormat.m_SampleRate = 22050; break;
        case RATE_11025_IDX: c.m_SoundFormat.m_SampleRate = 11025; break;
        default:             c.m_SoundFormat.m_SampleRate = 44100; break;
    }
    switch(editChannels->currentIndex()) {
        case CHANNELS_MONO_IDX:   c.m_SoundFormat.m_Channels = 1; break;
        case CHANNELS_STEREO_IDX: c.m_SoundFormat.m_Channels = 2; break;
        default:                  c.m_SoundFormat.m_Channels = 2; break;
    }
    switch(editSign->currentIndex()) {
        case SIGN_UNSIGNED_IDX: c.m_SoundFormat.m_IsSigned = false; break;
        case SIGN_SIGNED_IDX:   c.m_SoundFormat.m_IsSigned = true;  break;
        default:                c.m_SoundFormat.m_IsSigned = true; break;
    }
    switch(editEndianess->currentIndex()) {
        case ENDIAN_LITTLE_IDX: c.m_SoundFormat.m_Endianess = LITTLE_ENDIAN; break;
        case ENDIAN_BIG_IDX:    c.m_SoundFormat.m_Endianess = BIG_ENDIAN; break;
        default:                c.m_SoundFormat.m_Endianess = LITTLE_ENDIAN; break;
    }
    switch(editBits->currentIndex()) {
        case BITS_8_IDX:  c.m_SoundFormat.m_SampleBits = 8; break;
        case BITS_16_IDX: c.m_SoundFormat.m_SampleBits = 16; break;
        default:          c.m_SoundFormat.m_SampleBits = 16; break;
    }
    switch(editFileFormat->currentIndex()) {
        case FORMAT_WAV_IDX:  c.m_OutputFormat = RecordingConfig::outputWAV;  break;
        case FORMAT_AIFF_IDX: c.m_OutputFormat = RecordingConfig::outputAIFF; break;
        case FORMAT_AU_IDX:   c.m_OutputFormat = RecordingConfig::outputAU;   break;
        case FORMAT_RAW_IDX:  c.m_OutputFormat = RecordingConfig::outputRAW;  break;
#ifdef HAVE_LAME
        case FORMAT_MP3_IDX:  c.m_OutputFormat = RecordingConfig::outputMP3;  break;
#endif
#ifdef HAVE_OGG
        case FORMAT_OGG_IDX:  c.m_OutputFormat = RecordingConfig::outputOGG;  break;
#endif
        default:              c.m_OutputFormat = RecordingConfig::outputWAV;  break;
    }
#ifdef HAVE_LAME
    c.m_mp3Quality = editMP3Quality->value();
#endif
#ifdef HAVE_OGG
    c.m_oggQuality = ((float)editOggQuality->value()) / 9.0f;
#endif

    c.m_PreRecordingEnable  = m_checkboxPreRecordingEnable->isChecked();
    c.m_PreRecordingSeconds = m_spinboxPreRecordingSeconds->value();

    c.checkFormatSettings();
}


void RecordingConfiguration::slotCancel()
{
    if (m_dirty) {
        noticeRecordingConfigChanged(m_RecordingConfig);
        m_dirty = false;
    }
}


void RecordingConfiguration::slotFormatSelectionChanged()
{
    int bitsIDX   = editBits      ->currentIndex();
    int formatIDX = editFileFormat->currentIndex();

    int endianTest = 0x04030201;
    bool littleEndian = ((char*)&endianTest)[0] == 0x01;

#ifdef HAVE_LAME
    editMP3Quality ->setEnabled(false);
    labelMP3Quality->setEnabled(false);
#endif
#ifdef HAVE_OGG
    editOggQuality ->setEnabled(false);
    labelOggQuality->setEnabled(false);
#endif

    editBits->setEnabled(true);

    if (formatIDX == FORMAT_MP3_IDX) {
        editBits->setDisabled(true);
        editBits->setCurrentItem(BITS_16_IDX);
        editSign->setDisabled(true);
        editSign->setCurrentIndex(SIGN_SIGNED_IDX);
#ifdef HAVE_LAME
        editMP3Quality ->setEnabled(true);
        labelMP3Quality->setEnabled(true);
#endif
    } else if (formatIDX == FORMAT_OGG_IDX) {
        editBits->setDisabled(true);
        editBits->setCurrentItem(BITS_16_IDX);
        editSign->setDisabled(true);
        editSign->setCurrentIndex(SIGN_SIGNED_IDX);
#ifdef HAVE_OGG
        editOggQuality ->setEnabled(true);
        labelOggQuality->setEnabled(true);
#endif
    } else {
        if (bitsIDX == BITS_8_IDX) {
            if (formatIDX == FORMAT_RAW_IDX || formatIDX == FORMAT_AIFF_IDX) {
                editSign->setDisabled(false);
            } else {
                editSign->setDisabled(true);
                editSign->setCurrentIndex(formatIDX == FORMAT_WAV_IDX ? SIGN_UNSIGNED_IDX : SIGN_SIGNED_IDX);
            }
        } else {
            editSign->setDisabled(true);
            editSign->setCurrentIndex(SIGN_SIGNED_IDX);
        }
    }

    switch (formatIDX) {
        case FORMAT_RAW_IDX :
            editEndianess->setDisabled(false);
            break;
#ifdef HAVE_LAME
        case FORMAT_MP3_IDX :
            editEndianess->setCurrentIndex(littleEndian ? ENDIAN_LITTLE_IDX : ENDIAN_BIG_IDX);
            editEndianess->setDisabled(true);
            break;
#endif
#ifdef HAVE_OGG
        case FORMAT_OGG_IDX :
            editEndianess->setCurrentIndex(littleEndian ? ENDIAN_LITTLE_IDX : ENDIAN_BIG_IDX);
            editEndianess->setDisabled(true);
            break;
#endif
        default:
            editEndianess->setDisabled(true);
            if (formatIDX == FORMAT_AIFF_IDX || formatIDX == FORMAT_AU_IDX) {
                editEndianess->setCurrentIndex(ENDIAN_BIG_IDX);
            } else {
                editEndianess->setCurrentIndex(ENDIAN_LITTLE_IDX);
            }
            break;
    }
}



bool RecordingConfiguration::noticeEncoderBufferChanged (size_t BufferSize, size_t BufferCount)
{
    m_ignore_gui_updates = true;
    m_RecordingConfig.m_EncodeBufferSize  = BufferSize;
    m_RecordingConfig.m_EncodeBufferCount = BufferCount;
    setGUIBuffers(m_RecordingConfig);
    slotFormatSelectionChanged();
    m_ignore_gui_updates = false;
    return true;
}


bool RecordingConfiguration::noticeSoundFormatChanged (const SoundFormat &sf)
{
    m_ignore_gui_updates = true;
    m_RecordingConfig.m_SoundFormat = sf;
    setGUISoundFormat(m_RecordingConfig);
    slotFormatSelectionChanged();
    m_ignore_gui_updates = false;
    return true;
}


bool RecordingConfiguration::noticeMP3QualityChanged (int q)
{
    m_ignore_gui_updates = true;
    m_RecordingConfig.m_mp3Quality = q;
    setGUIEncoderQuality(m_RecordingConfig);
    slotFormatSelectionChanged();
    m_ignore_gui_updates = false;
    return true;
}

bool RecordingConfiguration::noticeOggQualityChanged (float q)
{
    m_ignore_gui_updates = true;
    m_RecordingConfig.m_oggQuality = q;
    setGUIEncoderQuality(m_RecordingConfig);
    slotFormatSelectionChanged();
    m_ignore_gui_updates = false;
    return true;
}

bool RecordingConfiguration::noticeRecordingDirectoryChanged(const QString &dir)
{
    m_ignore_gui_updates = true;
    m_RecordingConfig.m_Directory = dir;
    setGUIDirectories(m_RecordingConfig);
    slotFormatSelectionChanged();
    m_ignore_gui_updates = false;
    return true;
}

bool RecordingConfiguration::noticeOutputFormatChanged      (RecordingConfig::OutputFormat of)
{
    m_ignore_gui_updates = true;
    m_RecordingConfig.m_OutputFormat = of;
    setGUIOutputFormat(m_RecordingConfig);
    slotFormatSelectionChanged();
    m_ignore_gui_updates = false;
    return true;
}

bool RecordingConfiguration::noticePreRecordingChanged (bool enable, int seconds)
{
    m_ignore_gui_updates = true;
    m_RecordingConfig.m_PreRecordingEnable = enable;
    m_RecordingConfig.m_PreRecordingSeconds = seconds;
    setGUIPreRecording(m_RecordingConfig);
    m_ignore_gui_updates = false;
    return true;
}

bool RecordingConfiguration::noticeRecordingConfigChanged(const RecordingConfig &c)
{
    m_ignore_gui_updates = true;
    m_RecordingConfig = c;
    setGUIBuffers(c);
    setGUIDirectories(c);
    setGUISoundFormat(c);
    setGUIOutputFormat(c);
    setGUIEncoderQuality(c);
    setGUIPreRecording(c);
    slotFormatSelectionChanged();
    m_ignore_gui_updates = false;
    return true;
}

void RecordingConfiguration::slotSetDirty()
{
    if (!m_ignore_gui_updates) {
        m_dirty = true;
    }
}


#include "recording-configuration.moc"
