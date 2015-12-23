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

#include <QThread>
#include <QSemaphore>

#include <alsa/asoundlib.h>

#include "soundformat.h"
#include "thread-logging.h"

class AlsaSoundDevice;
class AlsaThread : public QThread,
                   public ThreadLogging
{
Q_OBJECT
public:
    AlsaThread (AlsaSoundDevice *parent, bool playback_not_capture, snd_pcm_t *handle, const SoundFormat &sf);
    ~AlsaThread();

    void                  run();

    bool                  error()       const { return m_error; }
    void                  resetError();

    void                  setDone();
    bool                  isDone() const { return m_done; }

    void                  setLatency(unsigned int us);

signals:
    void                  sigRequestPlaybackData();
    void                  sigCaptureDataAvailable();

protected:

    AlsaSoundDevice      *m_parent;
    bool                  m_playback_not_capture;
    snd_pcm_t            *m_alsa_handle;
    SoundFormat           m_soundFormat;

    bool                  m_error;
    bool                  m_done;

    unsigned int          m_latency_us;
};

#endif
