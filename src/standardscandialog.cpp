/***************************************************************************
                          standardscandialog.cpp  -  description
                             -------------------
    begin                : Son Aug 3 2003
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

#include "standardscandialog.h"
#include "ui_standardscandialog-ui.h"
#include "radiostation.h"

#include <QtGui/QProgressBar>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

#include <klocale.h>

#include <math.h>

StandardScanDialog::StandardScanDialog(QWidget *parent)
    : QDialog(parent),
      m_count(0),
      m_running(false),
      m_oldPowerOn(false),
      m_oldStation(NULL),
      m_ignorePower(false),
      m_ui(NULL),
      m_waiting4RDS(false)
{
    m_Wait4RDSTimeout.setInterval(2000); // wait at max 2 sec on RDS signal
    m_Wait4RDSTimeout.setSingleShot(true);
    QObject::connect(&m_Wait4RDSTimeout, SIGNAL(timeout()), this, SLOT(slotWait4RDSTimeout()));

    m_ui = new Ui_StandardScanDialogUI();
    m_ui->setupUi(this);
    m_ui->buttonStartStop->setText(i18n("&Start"));
    QObject::connect(m_ui->buttonOk,        SIGNAL(clicked()), this, SLOT(slotOk()));
    QObject::connect(m_ui->buttonStartStop, SIGNAL(clicked()), this, SLOT(slotStartStop()));
    QObject::connect(m_ui->buttonDiscard,   SIGNAL(clicked()), this, SLOT(slotDiscard()));
}


StandardScanDialog::~StandardScanDialog()
{
    stop();
    delete m_ui;
    m_ui = NULL;
}

bool StandardScanDialog::connectI (Interface *i)
{
    bool a = ISeekRadioClient::connectI(i);
    bool b = IErrorLogClient ::connectI(i);
    bool c = IRadioClient    ::connectI(i);

    return a || b || c;
}

bool StandardScanDialog::disconnectI (Interface *i)
{
    bool a = ISeekRadioClient::disconnectI(i);
    bool b = IErrorLogClient ::disconnectI(i);
    bool c = IRadioClient    ::disconnectI(i);

    return a || b || c;
}


void StandardScanDialog::start()
{
    if (!m_running) {
        m_running = true;
        m_stations.clearStations();
        m_startTime  = QDateTime::currentDateTime();
        m_oldPowerOn = queryIsPowerOn();
        m_oldStation = queryCurrentStation().copy();
        sendToBeginning();
        m_ignorePower = true;
        sendPowerOn();
        m_ignorePower = false;
        sendStartSeekUp();
        m_ui->buttonStartStop->setText(i18n("&Stop"));
        m_ui->buttonOk       ->hide();
        m_ui->buttonDiscard  ->hide();
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
        m_ui->buttonStartStop->setText(i18n("&Start"));
        m_ui->buttonOk       ->show();
        m_ui->buttonDiscard  ->show();
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
//         logDebug("StandardScanDialog::noticeSeekFinished");
        m_waiting4RDS = true;
        m_Wait4RDSTimeout.start();
    }
    else {
        continueScan();
    }

    return true;
}

bool StandardScanDialog::noticeSeekStopped ()
{
    if (rint(queryProgress() * 1000) >= 1000) {     // round to 4 digits
        stop();
    }
    return true;
}


bool StandardScanDialog::noticeProgress (float f)
{
    if (!m_running) return true;

    m_ui->progressBar->setValue((int)rint(f * 100));

    if (m_running) {
        int secs = m_startTime.secsTo(QDateTime::currentDateTime());
        int ms = (int)rint((1 - f) * (float) secs / f * 1000.0);

        if (ms > 0 && ms < 86400000)   // max one day
            m_ui->labelTime->setText(i18n("<p align=\"right\">%1</p>", QTime(0,0).addMSecs(ms).toString()));
        else
            m_ui->labelTime->setText(i18n("unknown"));

    } else {
        m_ui->labelTime->setText(i18n("unknown"));
    }
    return true;
}


void StandardScanDialog::slotStartStop()
{
    if (m_running) {
        stop();
    } else {
        start();
    }
}


void StandardScanDialog::slotOk()
{
    if (m_running) {
        stop();
    }
    accept();
}

void StandardScanDialog::slotDiscard()
{
    if (m_running) {
        stop();
    }
    reject();
}


void StandardScanDialog::continueScan()
{
    if (rint(queryProgress() * 1000) < 1000) {     // round to 4 digits
        if (m_running) {
//             logDebug("StandardScanDialog::sendStartSeekUp");
            sendStartSeekUp();
        }
    }
}


void StandardScanDialog::addCurrentStation()
{
//     logDebug("StandardScanDialog::addCurrentStation");
    ++m_count;
    QString s;
    s.setNum(m_count);

    RadioStation *st = queryCurrentStation().copy();
    if (!st->name().length()) {
        const QString &rds_name = queryRDSStationName();
        if (rds_name.length()) {
            st->setName(rds_name);
        }
        else {
            st->setName(i18n("new station %1", s));
        }
        st->setShortName(s);
        st->generateNewStationID();
    }

    int oldcount = m_stations.count();
    m_stations.addStation(*st);
    m_ui->listviewStations->appendStation(*st);

    if (oldcount != m_stations.count()) {
    } else {
        --m_count;
    }
    delete st;
}


void StandardScanDialog::slotWait4RDSTimeout()
{
    if (m_waiting4RDS) {
//         logDebug("StandardScanDialog::slotWait4RDSTimeout");
        m_waiting4RDS = false;
        addCurrentStation();
        continueScan();
    }
}


bool StandardScanDialog::noticeRDSStationNameChanged(const QString &)
{
    if (m_waiting4RDS) {
//         logDebug("StandardScanDialog::noticeRDSStationNameChanged");
        m_waiting4RDS = false;
        m_Wait4RDSTimeout.stop();
        addCurrentStation();
        continueScan();
    }
    return true;
}

#include "standardscandialog.moc"
