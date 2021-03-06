/***************************************************************************
                       oss-sound-configuration.cpp  -  description
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

#include <qcheckbox.h>

#include <kurlrequester.h>


#include "oss-sound-configuration.h"
#include "oss-sound.h"

OSSSoundConfiguration::OSSSoundConfiguration (QWidget *parent, OSSSoundDevice *dev)
 : OSSSoundConfigurationUI(parent),
   m_SoundDevice (dev),
   m_dirty(true),
   m_ignore_gui_updates(false)
{
    connect(editDSPDevice,      &KUrlRequester::textChanged,                 this, &OSSSoundConfiguration::slotSetDirty);
    connect(editMixerDevice,    &KUrlRequester::textChanged,                 this, &OSSSoundConfiguration::slotSetDirty);
    connect(editBufferSize,     QOverload<int>::of(&QSpinBox::valueChanged), this, &OSSSoundConfiguration::slotSetDirty);
    connect(chkDisablePlayback, &QCheckBox::toggled,                         this, &OSSSoundConfiguration::slotSetDirty);
    connect(chkDisableCapture,  &QCheckBox::toggled,                         this, &OSSSoundConfiguration::slotSetDirty);
    slotCancel();
}


OSSSoundConfiguration::~OSSSoundConfiguration ()
{
}


void OSSSoundConfiguration::slotOK()
{
    if (m_SoundDevice && m_dirty) {
        m_SoundDevice->setBufferSize     ( editBufferSize    ->value() * 1024);
        m_SoundDevice->enablePlayback    (!chkDisablePlayback->isChecked());
        m_SoundDevice->enableCapture     (!chkDisableCapture ->isChecked());
        m_SoundDevice->setDSPDeviceName  ( editDSPDevice     ->url());
        m_SoundDevice->setMixerDeviceName( editMixerDevice   ->url());
        m_dirty = false;
    }
}


void OSSSoundConfiguration::slotCancel()
{
    if (m_dirty) {
        m_ignore_gui_updates = true;
        editDSPDevice     ->setURL    (m_SoundDevice ?  m_SoundDevice->getDSPDeviceName()   : QString());
        editMixerDevice   ->setURL    (m_SoundDevice ?  m_SoundDevice->getMixerDeviceName() : QString());
        editBufferSize    ->setValue  (m_SoundDevice ?  m_SoundDevice->getBufferSize()/1024 : 4);
        chkDisablePlayback->setChecked(m_SoundDevice ? !m_SoundDevice->isPlaybackEnabled()  : false);
        chkDisableCapture ->setChecked(m_SoundDevice ? !m_SoundDevice->isCaptureEnabled()   : false);
        m_ignore_gui_updates = false;
        m_dirty = false;
    }
}

void OSSSoundConfiguration::slotUpdateConfig()
{
    slotSetDirty();
    slotCancel();
}

void OSSSoundConfiguration::slotSetDirty()
{
    if (!m_ignore_gui_updates) {
        m_dirty = true;
    }
}

