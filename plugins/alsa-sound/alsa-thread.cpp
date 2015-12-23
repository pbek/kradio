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

#include <QtGlobal>

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

    size_t min_fill_base = m_parent->getPlaybackBufferMinFill();
    size_t min_fill      = min_fill_base;
    int    frameSize     = m_soundFormat.frameSize();

    bool   needsPrepare  = true;

    while (!m_done && !m_error) {
        if (m_playback_not_capture) {
/*            if(!m_warning) {
                addWarningString(i18n("ALSA Thread: min fill = %1", min_fill));
            }*/

            while(!m_done && !m_error) {

                   // printf("Enter Locking Buffer Phase\n");
                m_parent->lockPlaybackBufferTransaction();
                   // printf("Entered Locking Buffer Phase\n");
                

                size_t  buffersize       = 0;
                size_t  maxAvailableSize = 0;
                char   *buffer           = m_parent->getPlaybackData(buffersize, maxAvailableSize);
                int     n_frames         = buffersize / frameSize;

                if (!buffersize || maxAvailableSize < min_fill) {
                    m_parent->unlockPlaybackBufferTransaction();
                      // printf("skipping pcm write: buffersize = %zi, min_fill = %zi\n", maxAvailableSize, min_fill);
                    emit sigRequestPlaybackData();
                    break;
                }
                m_parent->setWaitForMinPlaybackBufferFill(0/*percent*/);
//                 addWarningString(i18n("ALSA Thread: playing, bufsize = %1", buffersize));

                double softVolCorrectionFactor = 1;
                if (m_parent->getSoftPlaybackVolume(softVolCorrectionFactor)) {
                    double scale = m_parent->getSoftPlaybackVolumeValue() * softVolCorrectionFactor;
                    m_soundFormat.scaleSamples(buffer, scale, n_frames);
                }

                if (needsPrepare) {
                    // printf("pcm prepare\n");
                    snd_pcm_prepare(m_alsa_handle);
                    needsPrepare = false;
                }

                min_fill = 0; // once we start writing, no min fill any more
                   // printf("start pcm write: %i frames\n", n_frames);
                int      framesWritten = snd_pcm_writei(m_alsa_handle, buffer, n_frames);
                int      bytesWritten  = framesWritten * frameSize;
                   // printf("end pcm write: frames written = %i\n", framesWritten);

                if (framesWritten > 0) {
                    m_parent->freePlaybackData(bytesWritten);
                    ignoreUnderflow = false;
                }
                m_parent->unlockPlaybackBufferTransaction();
                   // printf("left buffer lock phase\n");

                if (framesWritten == 0) {
                    m_error = true;
                    log(ThreadLogging::LogError, i18n("ALSA Thread: cannot write data"));
                    break;
                } else if (framesWritten == -EAGAIN) {
                    break;
                } else if (framesWritten < 0 && !ignoreUnderflow){
                    min_fill     = min_fill_base;
                    needsPrepare = true;
                    log(ThreadLogging::LogWarning, i18n("ALSA Thread: buffer underrun"));
                    // printf("========================================================================\n");
                    // printf("=              UNDERFLOW!!!!!                                           \n");
                    // printf("========================================================================\n");
                }

                // for some reason, the blocking functionality of alsa seems to be suboptimum (causes high CPU load and system calls)
                // therefore let's wait for one alsa-period
                if (m_latency_us) {
                    // printf("alsa snd: waiting for %i us\n", m_latency_us);                  
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
                    emit sigCaptureDataAvailable();
                    break;
                }

                if (needsPrepare) {
                    snd_pcm_prepare(m_alsa_handle);
                    needsPrepare = false;
                }

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
                        // printf ("alsa recording thread: samples per read(%lli reads): min = %lli, max = %lli, avg = %f --- ", (long long)n_reads, (long long)min_frames_read, (long long)max_frames_read, (double)sum_frames_read / (double)n_reads);
                        // printf ("bytes: min = %lli, max = %lli, avg = %f\n", (long long)min_frames_read * frameSize, (long long)max_frames_read * frameSize, (double)sum_frames_read / (double)n_reads * frameSize);
                        n_reads         = 0;
                        sum_frames_read = 0;
                        min_frames_read = ~0;
                        max_frames_read = 0;
                    }
#endif
                }

                m_parent->unlockCaptureBufferTransaction();

                if (framesRead == 0) {
                    m_error = true;
                    log(ThreadLogging::LogError, i18n("AlsaThread: cannot read data"));
                    break;
                } else if (framesRead == -EAGAIN) {
                    break;
                } else if (framesRead < 0) {
                    needsPrepare = true;
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
        // printf ("long usleep\n");
        usleep(10000);
    }
    snd_pcm_prepare(m_alsa_handle);
}




void AlsaThread::resetError()
{
    m_error       = false;
}


void AlsaThread::setLatency(unsigned int us)
{
    m_latency_us = us; // > 1000 ? us : 1000; // let's define some minimum (1ms), although this 1ms might be far too low for reasonable purposes
}

