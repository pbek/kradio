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
#include "recording-context.h"
#include <kurlrequester.h>
#include <kcombobox.h>
#include <qspinbox.h>
#include <qlabel.h>


RecordingConfiguration::RecordingConfiguration (QWidget *parent)
    : RecordingConfigurationUI(parent)
{
    editDevice->setMode(KFile::File | KFile::ExistingOnly);
    editDirectory->setMode(KFile::Directory | KFile::ExistingOnly);

    QObject::connect(editFileFormat, SIGNAL(activated(int)),
                     this,           SLOT(slotFormatSelectionChanged()));
    QObject::connect(editBits,       SIGNAL(activated(int)),
                     this,           SLOT(slotFormatSelectionChanged()));

#ifndef HAVE_LAME_LAME_H
    editFileFormat->removeItem(FORMAT_MP3_IDX);
    delete editMP3Quality;
    editMP3Quality = NULL;
    delete labelMP3Quality;
    labelMP3Quality = NULL;
#endif
}


RecordingConfiguration::~RecordingConfiguration ()
{
}


bool RecordingConfiguration::noticeRecordingStarted()
{
    return false;
}


bool RecordingConfiguration::noticeMonitoringStarted()
{
    return false;
}


bool RecordingConfiguration::noticeRecordingStopped()
{
    return false;
}


bool RecordingConfiguration::noticeMonitoringStopped()
{
    return false;
}


bool RecordingConfiguration::noticeRecordingConfigChanged(const RecordingConfig &c)
{
    editBufferSize->setValue(c.encodeBufferSize / 1024);
    editBufferCount->setValue(c.encodeBufferCount);
    editDevice->setURL(c.device);
    editDirectory->setURL(c.directory);

    switch (c.bits) {
        case 8 : editBits->setCurrentItem(BITS_8_IDX ); break;
        case 16: editBits->setCurrentItem(BITS_16_IDX); break;
        default: editBits->setCurrentItem(BITS_16_IDX);
    }
    switch (c.channels) {
        case 1 : editChannels->setCurrentItem(CHANNELS_MONO_IDX); break;
        case 2 : editChannels->setCurrentItem(CHANNELS_STEREO_IDX); break;
        default: editChannels->setCurrentItem(CHANNELS_STEREO_IDX); break;
    }
    switch (c.sign) {
        case 0 : editSign->setCurrentItem(SIGN_UNSIGNED_IDX); break;
        case 1 : editSign->setCurrentItem(SIGN_SIGNED_IDX); break;
        default: editSign->setCurrentItem(SIGN_SIGNED_IDX); break;
    }
    switch (c.rate) {
        case 44100: editRate->setCurrentItem(RATE_44100_IDX); break;
        case 22050: editRate->setCurrentItem(RATE_22050_IDX); break;
        case 11025: editRate->setCurrentItem(RATE_11025_IDX); break;
        default:    editRate->setCurrentItem(RATE_44100_IDX); break;
    }
    switch (c.littleEndian) {
        case 0 : editEndianess->setCurrentItem(ENDIAN_BIG_IDX); break;
        case 1 : editEndianess->setCurrentItem(ENDIAN_LITTLE_IDX); break;
        default: editEndianess->setCurrentItem(ENDIAN_LITTLE_IDX); break;
    }
    switch (c.outputFormat) {
        case RecordingConfig::outputWAV:  editFileFormat->setCurrentItem(FORMAT_WAV_IDX);  break;
        case RecordingConfig::outputAIFF: editFileFormat->setCurrentItem(FORMAT_AIFF_IDX); break;
        case RecordingConfig::outputAU:   editFileFormat->setCurrentItem(FORMAT_AU_IDX);   break;
        case RecordingConfig::outputRAW:  editFileFormat->setCurrentItem(FORMAT_RAW_IDX);  break;
#ifdef HAVE_LAME_LAME_H
        case RecordingConfig::outputMP3:  editFileFormat->setCurrentItem(FORMAT_MP3_IDX);  break;
#endif
        default:                          editFileFormat->setCurrentItem(FORMAT_WAV_IDX);  break;
    }

#ifdef HAVE_LAME_LAME_H
    editMP3Quality->setValue(c.mp3Quality);
#endif

    slotFormatSelectionChanged();
    return true;
}


bool RecordingConfiguration::noticeRecordingContextChanged(const RecordingContext &)
{
    return false;
}



void RecordingConfiguration::slotOK()
{
    RecordingConfig c;

    c.encodeBufferSize  = editBufferSize->value() * 1024;
    c.encodeBufferCount = editBufferCount->value();

    c.device    = editDevice->url();
    c.directory = editDirectory->url();

    switch(editRate->currentItem()) {
        case RATE_44100_IDX: c.rate = 44100; break;
        case RATE_22050_IDX: c.rate = 22050; break;
        case RATE_11025_IDX: c.rate = 11025; break;
        default:             c.rate = 44100; break;
    }
    switch(editChannels->currentItem()) {
        case CHANNELS_MONO_IDX:   c.channels = 1; break;
        case CHANNELS_STEREO_IDX: c.channels = 2; break;
        default:                  c.channels = 2; break;
    }
    switch(editSign->currentItem()) {
        case SIGN_UNSIGNED_IDX: c.sign = false; break;
        case SIGN_SIGNED_IDX:   c.sign = true;  break;
        default:                c.sign = true; break;
    }
    switch(editEndianess->currentItem()) {
        case ENDIAN_LITTLE_IDX: c.littleEndian = true; break;
        case ENDIAN_BIG_IDX:    c.littleEndian = false; break;
        default:                c.littleEndian = true; break;
    }
    switch(editBits->currentItem()) {
        case BITS_8_IDX:  c.bits = 8; break;
        case BITS_16_IDX: c.bits = 16; break;
        default:          c.bits = 16; break;
    }
    switch(editFileFormat->currentItem()) {
        case FORMAT_WAV_IDX:  c.outputFormat = RecordingConfig::outputWAV;  break;
        case FORMAT_AIFF_IDX: c.outputFormat = RecordingConfig::outputAIFF; break;
        case FORMAT_AU_IDX:   c.outputFormat = RecordingConfig::outputAU;   break;
        case FORMAT_RAW_IDX:  c.outputFormat = RecordingConfig::outputRAW;  break;
#ifdef HAVE_LAME_LAME_H
        case FORMAT_MP3_IDX:  c.outputFormat = RecordingConfig::outputMP3;  break;
#endif
        default:              c.outputFormat = RecordingConfig::outputWAV;  break;
    }
#ifdef HAVE_LAME_LAME_H
    c.mp3Quality = editMP3Quality->value();
#endif

    c.checkFormatSettings();
    sendRecordingConfig(c);
}


void RecordingConfiguration::slotCancel()
{
    noticeRecordingConfigChanged(queryRecordingConfig());
}


void RecordingConfiguration::slotFormatSelectionChanged()
{
    int bitsIDX   = editBits->currentItem();
    int formatIDX = editFileFormat->currentItem();

    int endianTest = 0x04030201;
    bool littleEndian = ((char*)&endianTest)[0] == 0x01;

#ifdef HAVE_LAME_LAME_H
    if (formatIDX == FORMAT_MP3_IDX) {
        editBits->setDisabled(true);
        editBits->setCurrentItem(BITS_16_IDX);
        editSign->setDisabled(true);
        editSign->setCurrentItem(SIGN_SIGNED_IDX);
        editMP3Quality->setDisabled(false);
        labelMP3Quality->setEnabled(true);
    } else {
        editBits->setDisabled(false);
        editMP3Quality->setDisabled(true);
        labelMP3Quality->setEnabled(false);
#endif
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
#ifdef HAVE_LAME_LAME_H
    }
#endif
    switch (formatIDX) {
        case FORMAT_RAW_IDX :
            editEndianess->setDisabled(false);
            break;
#ifdef HAVE_LAME_LAME_H
        case FORMAT_MP3_IDX :
            editEndianess->setCurrentItem(littleEndian ? ENDIAN_LITTLE_IDX : ENDIAN_BIG_IDX);
            editEndianess->setDisabled(true);
            break;
#endif
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
