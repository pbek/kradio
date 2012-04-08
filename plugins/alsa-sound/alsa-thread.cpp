/***************************************************************************
                          alsa-thread.cpp  -  description
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

#include "alsa-thread.h"
#include "alsa-sound.h"

#include <QtCore/QtGlobal>

AlsaThread::AlsaThread (AlsaSoundDevice *parent, bool playback_not_capture, snd_pcm_t *handle, const SoundFormat &sf)
    : m_parent(parent),
      m_playback_not_capture(playback_not_capture),
      m_alsa_handle(handle),
      m_soundFormat(sf),
      m_error(false),
//       m_warning(false),
      m_done(false),
//       m_waitSemaphore(1),
      m_latency_us(0)
//       m_errwarnModifySemaphore(1)
{
//     waitForParent(); // aquire semaphore in order to guarantee that we do not start before the parent wants us to start.
}

AlsaThread::~AlsaThread()
{
}


/*void AlsaThread::awake()
{
    m_waitSemaphore.release();
}

void AlsaThread::waitForParent()
{
    m_waitSemaphore.acquire();
}
*/
void AlsaThread::setDone()
{
    m_done = true;
//     awake();
}

void AlsaThread::run()
{
    m_done                   = false;
    bool     ignoreUnderflow = true;
#ifdef __ALSA_RECORDING_BUFFER_DEBUGGING__
    size_t   sum_frames_read = 0;
    size_t   min_frames_read = ~0;
    size_t   max_frames_read = 0;
    size_t   n_reads         = 0;
#endif

    while (!m_done && !m_error) {
        if (m_playback_not_capture) {
            size_t min_fill = m_parent->getPlaybackBufferMinFill();
/*            if(!m_warning) {
                addWarningString(i18n("ALSA Thread: min fill = %1", min_fill));
            }*/
            m_parent->setWaitForMinPlaybackBufferFill(66/*percent*/);

            while(!m_done && !m_error) {

                m_parent->lockPlaybackBufferTransaction();

                size_t  buffersize = 0;
                char   *buffer     = m_parent->getPlaybackData(buffersize);

                if (!buffersize || buffersize < min_fill) {
                    m_parent->unlockPlaybackBufferTransaction();
                    break;
                }
                m_parent->setWaitForMinPlaybackBufferFill(0/*percent*/);
//                 addWarningString(i18n("ALSA Thread: playing, bufsize = %1", buffersize));

                int      frameSize     = m_soundFormat.frameSize();
                int      framesWritten = snd_pcm_writei(m_alsa_handle, buffer, buffersize / frameSize);
                int      bytesWritten  = framesWritten * frameSize;


                if (framesWritten > 0) {
                    m_parent->freePlaybackData(bytesWritten);
                    ignoreUnderflow = false;
                }
                m_parent->unlockPlaybackBufferTransaction();

                if (framesWritten == 0) {
                    m_error = true;
                    log(ThreadLogging::LogError, i18n("ALSA Thread: cannot write data"));
                    break;
                } else if (framesWritten == -EAGAIN) {
                    break;
                } else if (framesWritten < 0 && !ignoreUnderflow){
                    m_parent->setWaitForMinPlaybackBufferFill(66/*percent*/);
                    snd_pcm_prepare(m_alsa_handle);
                    log(ThreadLogging::LogWarning, i18n("ALSA Thread: buffer underrun"));
                }

                // for some reason, the blocking functionality of alsa seems to be suboptimum (causes high CPU load and system calls)
                // therefore let's wait for one alsa-period
//                 printf("alsa snd: waiting for %i us\n", m_latency_us);
                if (m_latency_us) {
                    usleep(m_latency_us);
                }
            }
        }
        else {
            while (!m_done && !m_error) {

                m_parent->lockCaptureBufferTransaction();

                size_t  bufsize = 0;
                char   *buffer  = m_parent->getFreeCaptureBuffer(bufsize);

                if (!bufsize) {
                    m_parent->unlockCaptureBufferTransaction();
                    break;
                }

                size_t frameSize  = m_soundFormat.frameSize();
                int    framesRead = snd_pcm_readi(m_alsa_handle, buffer, bufsize / frameSize);
                size_t bytesRead  = framesRead > 0 ? framesRead * frameSize : 0;

                if (framesRead > 0) {
                    m_parent->removeFreeCaptureBufferSpace(bytesRead);

#ifdef __ALSA_RECORDING_BUFFER_DEBUGGING__
                    sum_frames_read += framesRead;
                    min_frames_read = qMin((size_t)framesRead, min_frames_read);
                    max_frames_read = qMax((size_t)framesRead, max_frames_read);
                    n_reads++;
                    if (n_reads >= 100) {
                        printf ("alsa recording thread: samples per read(%lli reads): min = %lli, max = %lli, avg = %f --- ", (long long)n_reads, (long long)min_frames_read, (long long)max_frames_read, (double)sum_frames_read / (double)n_reads);
                        printf ("bytes: min = %lli, max = %lli, avg = %f\n", (long long)min_frames_read * frameSize, (long long)max_frames_read * frameSize, (double)sum_frames_read / (double)n_reads * frameSize);
                        n_reads         = 0;
                        sum_frames_read = 0;
                        min_frames_read = ~0;
                        max_frames_read = 0;
                    }
#endif
                }

                m_parent->unlockCaptureBufferTransaction();

                if (framesRead == 0) {
                    snd_pcm_prepare(m_alsa_handle);
                    m_error = true;
                    log(ThreadLogging::LogError, i18n("AlsaThread: cannot read data"));
                    break;
                } else if (framesRead == -EAGAIN) {
                    break;
                } else if (framesRead < 0) {
                    snd_pcm_prepare(m_alsa_handle);
                    log(ThreadLogging::LogWarning, i18n("AlsaThread: buffer overrun"));
                }

                // for some reason, the blocking functionality of alsa seems to be suboptimum (causes high CPU load and system calls)
                // therefore let's wait for one alsa-period
                if (m_latency_us) {
                    usleep(m_latency_us);
                }
            }
        }
        // happens if there is some error or no new sound data for playing or recording
        usleep(10000);
    }
}




void AlsaThread::resetError()
{
    m_error       = false;
}


void AlsaThread::setLatency(unsigned int us)
{
    m_latency_us = us; // > 1000 ? us : 1000; // let's define some minimum (1ms), although this 1ms might be far too low for reasonable purposes
}

