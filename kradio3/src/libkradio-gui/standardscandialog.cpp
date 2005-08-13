/***************************************************************************
                          standardscandialog.cpp  -  description
                             -------------------
    begin                : Son Aug 3 2003
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

#include "standardscandialog.h"
#include "../radio-stations/radiostation.h"

#include <qprogressbar.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <klocale.h>

#include <math.h>

StandardScanDialog::StandardScanDialog(QWidget *parent)
    : StandardScanDialogUI(parent, NULL, true),
      m_count(0),
      m_running(false),
      m_oldPowerOn(false),
      m_oldStation(NULL),
      m_ignorePower(false)
{
    QObject::connect(buttonCancel, SIGNAL(clicked()), this, SLOT(slotCancelDone()));
}


StandardScanDialog::~StandardScanDialog()
{
    stop();
}

bool StandardScanDialog::connectI (Interface *i)
{
    bool a = ISeekRadioClient::connectI(i);
//    bool b = IRadioSoundClient::connectI(i);
    bool c = IRadioClient::connectI(i);

    return a || /*b ||*/ c;
}

bool StandardScanDialog::disconnectI (Interface *i)
{
    bool a = ISeekRadioClient::disconnectI(i);
//    bool b = IRadioSoundClient::disconnectI(i);
    bool c = IRadioClient::disconnectI(i);

    return a || /*b ||*/ c;
}


void StandardScanDialog::start()
{
    if (!m_running) {
        m_running = true;
        m_stations.all().clear();
        m_startTime = QDateTime::currentDateTime();
        m_oldPowerOn = queryIsPowerOn();
        m_oldStation = queryCurrentStation().copy();
        sendToBeginning();
        m_ignorePower = true;
        sendPowerOn();
        m_ignorePower = false;
        sendStartSeekUp();
    }
}


void StandardScanDialog::stop()
{
    if (m_running) {
        m_running = false;

        sendStopSeek();
        if (!m_oldPowerOn) sendPowerOff();
        sendActivateStation(*m_oldStation);
        delete m_oldStation;
        m_oldStation = NULL;
    }
}


bool StandardScanDialog::noticePowerChanged(bool on)
{
    if (!on && !m_ignorePower) {
        stop();
    }
    return true;
}

bool StandardScanDialog::noticeSeekStarted (bool /*up*/)
{
    return false;
}

bool StandardScanDialog::noticeSeekFinished (const RadioStation &, bool goodQuality)
{
    if (goodQuality) {
        ++m_count;
        QString s;
        s.setNum(m_count);

        RadioStation *st = queryCurrentStation().copy();
        if (st->name().isNull()) {
            st->setName(i18n("new station ") + s);
            st->setShortName(s);
            st->generateNewStationID();
        }

        int oldcount = m_stations.count();
        m_stations.all().append(st);

        if (oldcount != m_stations.count()) {
        } else {
            --m_count;
        }
        delete st;
    }

    if (rint(queryProgress() * 1000) < 1000) {     // round to 4 digits
        if (m_running) sendStartSeekUp();
    }
    return true;
}

bool StandardScanDialog::noticeSeekStopped ()
{
    if (rint(queryProgress() * 1000) >= 1000) {     // round to 4 digits
        buttonCancel->setText(i18n("&Done"));
        stop();
    }
    return true;
}


bool StandardScanDialog::noticeProgress (float f)
{
    if (!m_running) return true;

    progressBar->setProgress((int)rint(f * 100));

    if (m_running) {
        int secs = m_startTime.secsTo(QDateTime::currentDateTime());
        int ms = (int)rint((1 - f) * (float) secs / f * 1000.0);

        if (ms > 0 && ms < 86400000)   // max one day
            labelTime->setText("<p align=\"right\">" + QTime(0,0).addMSecs(ms).toString() + "</p>");
        else
            labelTime->setText(i18n("unknown"));

    } else {
        labelTime->setText(i18n("unknown"));
    }
    return true;
}


void StandardScanDialog::slotCancelDone()
{
    if (m_running) {
        stop();
        reject();
    } else {
        accept();
    }
}



#include <standardscandialog.moc>
