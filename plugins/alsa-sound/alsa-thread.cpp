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

AlsaThread::AlsaThread (AlsaSoundDevice *parent, bool playback_not_capture, snd_pcm_t *handle, const SoundFormat &sf)
    : m_parent(parent),
      m_playback_not_capture(playback_not_capture),
      m_alsa_handle(handle),
      m_soundFormat(sf),
      m_error(false),
      m_warning(false),
      m_done(false),
//       m_waitSemaphore(1),
      m_errwarnModifySemaphore(1)
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
    m_done = false;
    bool ignoreUnderflow = true;
    while (!m_done && !m_error) {
        if (m_playback_not_capture) {
            size_t min_fill = m_parent->getPlaybackBufferMinFill();
/*            if(!m_warning) {
                addWarningString(i18n("ALSA Thread: min fill = %1", min_fill));
            }*/
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
                    addErrorString(i18n("ALSA Thread: cannot write data"));
                    break;
                } else if (framesWritten == -EAGAIN) {
                    break;
                } else if (framesWritten < 0 && !ignoreUnderflow){
                    m_parent->setWaitForMinPlaybackBufferFill(66/*percent*/);
                    snd_pcm_prepare(m_alsa_handle);
                    addWarningString(i18n("ALSA Thread: buffer underrun"));
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
                }

                m_parent->unlockCaptureBufferTransaction();

                if (framesRead == 0) {
                    snd_pcm_prepare(m_alsa_handle);
                    addErrorString(i18n("AlsaThread: cannot read data"));
                    break;
                } else if (framesRead == -EAGAIN) {
                    break;
                } else if (framesRead < 0) {
                    snd_pcm_prepare(m_alsa_handle);
                    addWarningString(i18n("AlsaThread: buffer overrun"));
                }

            }
        }
        usleep(5000);
    }
}




void AlsaThread::addErrorString(const QString &s)
{
    m_errwarnModifySemaphore.acquire();
    m_error = true;
    if (m_errorString.length()) {
        m_errorString += " \n";
    }
    m_errorString += s;
    m_errwarnModifySemaphore.release();
}

void AlsaThread::addWarningString(const QString &s)
{
    m_errwarnModifySemaphore.acquire();
    m_warning = true;
    if (m_warningString.length()) {
        m_warningString += " \n";
    }
    m_warningString += s;
    m_errwarnModifySemaphore.release();
}

void AlsaThread::resetWarning()
{
    m_errwarnModifySemaphore.acquire();
    m_warning       = false;
    m_warningString = "";
    m_errwarnModifySemaphore.release();
}

void AlsaThread::resetError()
{
    m_errwarnModifySemaphore.acquire();
    m_error       = false;
    m_errorString = "";
    m_errwarnModifySemaphore.release();
}

QString AlsaThread::warningString() const
{
    m_errwarnModifySemaphore.acquire();
    QString ret = QString(m_warningString.constData(), m_warningString.length()); // deep copy
    m_errwarnModifySemaphore.release();
    return ret;
}


QString AlsaThread::errorString() const
{
    m_errwarnModifySemaphore.acquire();
    QString ret = QString(m_errorString.constData(), m_errorString.length()); // deep copy
    m_errwarnModifySemaphore.release();
    return ret;
}