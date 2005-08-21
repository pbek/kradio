/***************************************************************************
                          recording-configuration.cpp  -  description
                             -------------------
    begin                : So Aug 31 2003
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

#include "recording-configuration.h"
//#include "recording-context.h"

#include <kurlrequester.h>
#include <kcombobox.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qcheckbox.h>


RecordingConfiguration::RecordingConfiguration (QWidget *parent)
    : RecordingConfigurationUI(parent)
{
    editDirectory->setMode(KFile::Directory | KFile::ExistingOnly);

    QObject::connect(editFileFormat, SIGNAL(activated(int)),
                     this,           SLOT(slotFormatSelectionChanged()));
    QObject::connect(editBits,       SIGNAL(activated(int)),
                     this,           SLOT(slotFormatSelectionChanged()));

#ifndef HAVE_LAME_LAME_H
    editFileFormat->removeItem(FORMAT_MP3_IDX_ORG);
    delete editMP3Quality;
    editMP3Quality = NULL;
    delete labelMP3Quality;
    labelMP3Quality = NULL;
#endif
#if !defined(HAVE_VORBIS_VORBISENC_H) || !defined(HAVE_OGG_OGG_H)
    editFileFormat->removeItem(FORMAT_OGG_IDX_ORG);
    delete editOggQuality;
    editOggQuality = NULL;
    delete labelOggQuality;
    labelOggQuality = NULL;
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
    editDirectory->setURL(c.m_Directory);
}

void RecordingConfiguration::setGUISoundFormat(const RecordingConfig &c)
{
    switch (c.m_SoundFormat.m_SampleBits) {
        case 8 : editBits->setCurrentItem(BITS_8_IDX ); break;
        case 16: editBits->setCurrentItem(BITS_16_IDX); break;
        default: editBits->setCurrentItem(BITS_16_IDX);
    }
    switch (c.m_SoundFormat.m_Channels) {
        case 1 : editChannels->setCurrentItem(CHANNELS_MONO_IDX); break;
        case 2 : editChannels->setCurrentItem(CHANNELS_STEREO_IDX); break;
        default: editChannels->setCurrentItem(CHANNELS_STEREO_IDX); break;
    }
    switch (c.m_SoundFormat.m_IsSigned) {
        case 0 : editSign->setCurrentItem(SIGN_UNSIGNED_IDX); break;
        case 1 : editSign->setCurrentItem(SIGN_SIGNED_IDX); break;
        default: editSign->setCurrentItem(SIGN_SIGNED_IDX); break;
    }
    switch (c.m_SoundFormat.m_SampleRate) {
        case 44100: editRate->setCurrentItem(RATE_44100_IDX); break;
        case 22050: editRate->setCurrentItem(RATE_22050_IDX); break;
        case 11025: editRate->setCurrentItem(RATE_11025_IDX); break;
        default:    editRate->setCurrentItem(RATE_44100_IDX); break;
    }
    switch (c.m_SoundFormat.m_Endianess) {
        case BIG_ENDIAN    : editEndianess->setCurrentItem(ENDIAN_BIG_IDX); break;
        case LITTLE_ENDIAN : editEndianess->setCurrentItem(ENDIAN_LITTLE_IDX); break;
        default: editEndianess->setCurrentItem(ENDIAN_LITTLE_IDX); break;
    }
}

void RecordingConfiguration::setGUIOutputFormat(const RecordingConfig &c)
{
    switch (c.m_OutputFormat) {
        case RecordingConfig::outputWAV:  editFileFormat->setCurrentItem(FORMAT_WAV_IDX);  break;
        case RecordingConfig::outputAIFF: editFileFormat->setCurrentItem(FORMAT_AIFF_IDX); break;
        case RecordingConfig::outputAU:   editFileFormat->setCurrentItem(FORMAT_AU_IDX);   break;
        case RecordingConfig::outputRAW:  editFileFormat->setCurrentItem(FORMAT_RAW_IDX);  break;
#ifdef HAVE_LAME_LAME_H
        case RecordingConfig::outputMP3:  editFileFormat->setCurrentItem(FORMAT_MP3_IDX);  break;
#endif
#if defined(HAVE_VORBIS_VORBISENC_H) && defined(HAVE_OGG_OGG_H)
        case RecordingConfig::outputOGG:  editFileFormat->setCurrentItem(FORMAT_OGG_IDX);  break;
#endif
        default:                          editFileFormat->setCurrentItem(FORMAT_WAV_IDX);  break;
    }
}

void RecordingConfiguration::setGUIEncoderQuality(const RecordingConfig &c)
{
#ifdef HAVE_LAME_LAME_H
    editMP3Quality->setValue(c.m_mp3Quality);
#endif
#if defined(HAVE_VORBIS_VORBISENC_H) && defined(HAVE_OGG_OGG_H)
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
    storeConfig();
    sendRecordingConfig(m_RecordingConfig);
}


void RecordingConfiguration::storeConfig()
{
    RecordingConfig &c = m_RecordingConfig;

    c.m_EncodeBufferSize  = editBufferSize->value() * 1024;
    c.m_EncodeBufferCount = editBufferCount->value();

    c.m_Directory = editDirectory->url();

    switch(editRate->currentItem()) {
        case RATE_44100_IDX: c.m_SoundFormat.m_SampleRate = 44100; break;
        case RATE_22050_IDX: c.m_SoundFormat.m_SampleRate = 22050; break;
        case RATE_11025_IDX: c.m_SoundFormat.m_SampleRate = 11025; break;
        default:             c.m_SoundFormat.m_SampleRate = 44100; break;
    }
    switch(editChannels->currentItem()) {
        case CHANNELS_MONO_IDX:   c.m_SoundFormat.m_Channels = 1; break;
        case CHANNELS_STEREO_IDX: c.m_SoundFormat.m_Channels = 2; break;
        default:                  c.m_SoundFormat.m_Channels = 2; break;
    }
    switch(editSign->currentItem()) {
        case SIGN_UNSIGNED_IDX: c.m_SoundFormat.m_IsSigned = false; break;
        case SIGN_SIGNED_IDX:   c.m_SoundFormat.m_IsSigned = true;  break;
        default:                c.m_SoundFormat.m_IsSigned = true; break;
    }
    switch(editEndianess->currentItem()) {
        case ENDIAN_LITTLE_IDX: c.m_SoundFormat.m_Endianess = LITTLE_ENDIAN; break;
        case ENDIAN_BIG_IDX:    c.m_SoundFormat.m_Endianess = BIG_ENDIAN; break;
        default:                c.m_SoundFormat.m_Endianess = LITTLE_ENDIAN; break;
    }
    switch(editBits->currentItem()) {
        case BITS_8_IDX:  c.m_SoundFormat.m_SampleBits = 8; break;
        case BITS_16_IDX: c.m_SoundFormat.m_SampleBits = 16; break;
        default:          c.m_SoundFormat.m_SampleBits = 16; break;
    }
    switch(editFileFormat->currentItem()) {
        case FORMAT_WAV_IDX:  c.m_OutputFormat = RecordingConfig::outputWAV;  break;
        case FORMAT_AIFF_IDX: c.m_OutputFormat = RecordingConfig::outputAIFF; break;
        case FORMAT_AU_IDX:   c.m_OutputFormat = RecordingConfig::outputAU;   break;
        case FORMAT_RAW_IDX:  c.m_OutputFormat = RecordingConfig::outputRAW;  break;
#ifdef HAVE_LAME_LAME_H
        case FORMAT_MP3_IDX:  c.m_OutputFormat = RecordingConfig::outputMP3;  break;
#endif
#if defined(HAVE_VORBIS_VORBISENC_H) && defined(HAVE_OGG_OGG_H)
        case FORMAT_OGG_IDX:  c.m_OutputFormat = RecordingConfig::outputOGG;  break;
#endif
        default:              c.m_OutputFormat = RecordingConfig::outputWAV;  break;
    }
#ifdef HAVE_LAME_LAME_H
    c.m_mp3Quality = editMP3Quality->value();
#endif
#if defined(HAVE_VORBIS_VORBISENC_H) && defined(HAVE_OGG_OGG_H)
    c.m_oggQuality = ((float)editOggQuality->value()) / 9.0f;
#endif

    c.m_PreRecordingEnable  = m_checkboxPreRecordingEnable->isChecked();
    c.m_PreRecordingSeconds = m_spinboxPreRecordingSeconds->value();

    c.checkFormatSettings();
}


void RecordingConfiguration::slotCancel()
{
    noticeRecordingConfigChanged(m_RecordingConfig);
}


void RecordingConfiguration::slotFormatSelectionChanged()
{
    int bitsIDX   = editBits->currentItem();
    int formatIDX = editFileFormat->currentItem();

    int endianTest = 0x04030201;
    bool littleEndian = ((char*)&endianTest)[0] == 0x01;

#ifdef HAVE_LAME_LAME_H
    editMP3Quality ->setEnabled(false);
    labelMP3Quality->setEnabled(false);
#endif
#if defined(HAVE_VORBIS_VORBISENC_H) && defined(HAVE_OGG_OGG_H)
    editOggQuality ->setEnabled(false);
    labelOggQuality->setEnabled(false);
#endif

    editBits->setEnabled(true);

    if (formatIDX == FORMAT_MP3_IDX) {
        editBits->setDisabled(true);
        editBits->setCurrentItem(BITS_16_IDX);
        editSign->setDisabled(true);
        editSign->setCurrentItem(SIGN_SIGNED_IDX);
#ifdef HAVE_LAME_LAME_H
        editMP3Quality ->setEnabled(true);
        labelMP3Quality->setEnabled(true);
#endif
    } else if (formatIDX == FORMAT_OGG_IDX) {
        editBits->setDisabled(true);
        editBits->setCurrentItem(BITS_16_IDX);
        editSign->setDisabled(true);
        editSign->setCurrentItem(SIGN_SIGNED_IDX);
#if defined(HAVE_VORBIS_VORBISENC_H) && defined(HAVE_OGG_OGG_H)
        editOggQuality ->setEnabled(true);
        labelOggQuality->setEnabled(true);
#endif
    } else {
        if (bitsIDX == BITS_8_IDX) {
            if (formatIDX == FORMAT_RAW_IDX || formatIDX == FORMAT_AIFF_IDX) {
                editSign->setDisabled(false);
            } else {
                editSign->setDisabled(true);
                editSign->setCurrentItem(formatIDX == FORMAT_WAV_IDX ? SIGN_UNSIGNED_IDX : SIGN_SIGNED_IDX);
            }
        } else {
            editSign->setDisabled(true);
            editSign->setCurrentItem(SIGN_SIGNED_IDX);
        }
    }

    switch (formatIDX) {
        case FORMAT_RAW_IDX :
            editEndianess->setDisabled(false);
            break;
        case FORMAT_MP3_IDX :
            editEndianess->setCurrentItem(littleEndian ? ENDIAN_LITTLE_IDX : ENDIAN_BIG_IDX);
            editEndianess->setDisabled(true);
            break;
        case FORMAT_OGG_IDX :
            editEndianess->setCurrentItem(littleEndian ? ENDIAN_LITTLE_IDX : ENDIAN_BIG_IDX);
            editEndianess->setDisabled(true);
            break;
        default:
            editEndianess->setDisabled(true);
            if (formatIDX == FORMAT_AIFF_IDX || formatIDX == FORMAT_AU_IDX) {
                editEndianess->setCurrentItem(ENDIAN_BIG_IDX);
            } else {
                editEndianess->setCurrentItem(ENDIAN_LITTLE_IDX);
            }
            break;
    }
}



bool RecordingConfiguration::noticeEncoderBufferChanged (size_t BufferSize, size_t BufferCount)
{
    m_RecordingConfig.m_EncodeBufferSize  = BufferSize;
    m_RecordingConfig.m_EncodeBufferCount = BufferCount;
    setGUIBuffers(m_RecordingConfig);
    slotFormatSelectionChanged();
    return true;
}


bool RecordingConfiguration::noticeSoundFormatChanged (const SoundFormat &sf)
{
    m_RecordingConfig.m_SoundFormat = sf;
    setGUISoundFormat(m_RecordingConfig);
    slotFormatSelectionChanged();
    return true;
}


bool RecordingConfiguration::noticeMP3QualityChanged (int q)
{
    m_RecordingConfig.m_mp3Quality = q;
    setGUIEncoderQuality(m_RecordingConfig);
    slotFormatSelectionChanged();
    return true;
}

bool RecordingConfiguration::noticeOggQualityChanged (float q)
{
    m_RecordingConfig.m_oggQuality = q;
    setGUIEncoderQuality(m_RecordingConfig);
    slotFormatSelectionChanged();
    return true;
}

bool RecordingConfiguration::noticeRecordingDirectoryChanged(const QString &dir)
{
    m_RecordingConfig.m_Directory = dir;
    setGUIDirectories(m_RecordingConfig);
    slotFormatSelectionChanged();
    return true;
}

bool RecordingConfiguration::noticeOutputFormatChanged      (RecordingConfig::OutputFormat of)
{
    m_RecordingConfig.m_OutputFormat = of;
    setGUIOutputFormat(m_RecordingConfig);
    slotFormatSelectionChanged();
    return true;
}

bool RecordingConfiguration::noticePreRecordingChanged (bool enable, int seconds)
{
    m_RecordingConfig.m_PreRecordingEnable = enable;
    m_RecordingConfig.m_PreRecordingSeconds = seconds;
    setGUIPreRecording(m_RecordingConfig);
    return true;
}

bool RecordingConfiguration::noticeRecordingConfigChanged(const RecordingConfig &c)
{
    m_RecordingConfig = c;
    setGUIBuffers(c);
    setGUIDirectories(c);
    setGUISoundFormat(c);
    setGUIOutputFormat(c);
    setGUIEncoderQuality(c);
    setGUIPreRecording(c);
    slotFormatSelectionChanged();
    return true;
}




#include "recording-configuration.moc"
