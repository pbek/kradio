/***************************************************************************
                          alsa-thread.h  -  description
                             -------------------
    begin                : Thu May 26 2005
    copyright            : (C) 2005 by Martin Witte
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

#ifndef __ALSA_THREAD_H__
#define __ALSA_THREAD_H__

#include <QtCore/QThread>
#include <QtCore/QSemaphore>

#include <alsa/asoundlib.h>

#include "soundformat.h"

class AlsaSoundDevice;
class AlsaThread : public QThread
{
public:
    AlsaThread (AlsaSoundDevice *parent, bool playback_not_capture, snd_pcm_t *handle, const SoundFormat &sf);
    ~AlsaThread();

    void                  run();

    bool                  error()       const { return m_error; }
    QString               errorString() const;
    void                  resetError();

    bool                  warning()       const { return m_warning; }
    QString               warningString() const;
    void                  resetWarning();

    void                  setDone();
    bool                  isDone() const { return m_done; }

//     void                  awake();
protected:

//     void                  waitForParent();
    void                  addErrorString(const QString &s);
    void                  addWarningString(const QString &s);


protected:

    AlsaSoundDevice      *m_parent;
    bool                  m_playback_not_capture;
    snd_pcm_t            *m_alsa_handle;
    SoundFormat           m_soundFormat;

    bool                  m_error;
    QString               m_errorString;
    bool                  m_warning;
    QString               m_warningString;
    bool                  m_done;


//     QSemaphore            m_waitSemaphore;
    mutable QSemaphore    m_errwarnModifySemaphore;
};

#endif