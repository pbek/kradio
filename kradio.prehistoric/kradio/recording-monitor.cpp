/***************************************************************************
                          recording-monitor.cpp  -  description
                             -------------------
    begin                : Mo Sep 1 2003
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

#include "recording-monitor.h"
#include "recording-context.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <kconfig.h>

RecordingMonitor::RecordingMonitor(QWidget *parent, const QString &name)
  : QWidget(parent, (const char*)name),
    WidgetPluginBase(name),
    m_showHideOnStartStop(true)
{
	setCaption(i18n("Recording Monitor"));
	QGridLayout *l = new QGridLayout(this, 7, 2, 10, 4);

	l->addWidget(                  new QLabel(i18n("Status"),         this), 0, 0);
	l->addWidget(m_labelStatus   = new QLabel(i18n("<undefined>"),    this), 0, 1);
	l->addWidget(                  new QLabel(i18n("Recording File"), this), 1, 0);
	l->addWidget(m_labelFileName = new QLabel(i18n("<undefined>"),    this), 1, 1);
	l->addWidget(                  new QLabel(i18n("File Size"),      this), 2, 0);
	l->addWidget(m_labelSize     = new QLabel(i18n("<undefined>"),    this), 2, 1);
	l->addWidget(                  new QLabel(i18n("Recording Time"), this), 3, 0);
	l->addWidget(m_labelTime     = new QLabel(i18n("<undefined>"),    this), 3, 1);
	l->addWidget(                  new QLabel(i18n("Sample Rate"),    this), 4, 0);
	l->addWidget(m_labelRate     = new QLabel(i18n("<undefined>"),    this), 4, 1);

	QPushButton *close     = new QPushButton("&Close", this);
	m_btnStartStop         = new QPushButton("&Start", this);
	QObject::connect(close, SIGNAL(clicked()), this, SLOT(hide()));
	QObject::connect(m_btnStartStop, SIGNAL(clicked()), this, SLOT(slotStartStopRecording()));

	m_showHide = new QCheckBox(i18n("Hide/Show when recording stops/starts"), this);
	QObject::connect(m_showHide, SIGNAL(toggled(bool)), this, SLOT(slotHideShowToggle(bool)));
	m_showHide->setChecked(m_showHideOnStartStop);

	QHBoxLayout *hl1 = new QHBoxLayout(l);
	l->addMultiCell(hl1, 5, 5, 0, 1);
	hl1->addItem(new QSpacerItem(10, 1));
	hl1->addWidget(m_showHide);
	hl1->addItem(new QSpacerItem(10, 1));

	QHBoxLayout *hl2 = new QHBoxLayout(l);
	l->addMultiCell(hl2, 6, 6, 0, 1);
//	hl2->addItem(new QSpacerItem(10, 1));
	hl2->addWidget(close);
	hl2->addWidget(m_btnStartStop);
//	hl2->addItem(new QSpacerItem(10, 1));
}


RecordingMonitor::~RecordingMonitor()
{
}

// WidgetPluginBase

void   RecordingMonitor::saveState (KConfig *config) const
{
    config->setGroup(QString("recordingmonitor-") + name());

	WidgetPluginBase::saveState(config);
	config->writeEntry("showHideOnStartStop", m_showHideOnStartStop);
}


void   RecordingMonitor::restoreState (KConfig *config)
{
	config->setGroup(QString("recordingmonitor-") + name());

	WidgetPluginBase::restoreState(config);
	m_showHideOnStartStop = config->readBoolEntry("showHideOnStartStop", true);
	m_showHide->setChecked(m_showHideOnStartStop);
}


bool   RecordingMonitor::connect(Interface *i)
{
	bool a = IRecordingClient::connect(i);
	return a;
}

bool   RecordingMonitor::disconnect(Interface *i)
{
	bool a = IRecordingClient::disconnect(i);
	return a;
}

ConfigPageInfo  RecordingMonitor::createConfigurationPage()
{
	return ConfigPageInfo();
}

QWidget        *RecordingMonitor::createAboutPage()
{
	// FIXME
	return NULL;
}


// IRecordingClient

bool RecordingMonitor::noticeRecordingStarted()
{
	if (m_showHideOnStartStop)
		show();
	m_btnStartStop->setText(i18n("&Stop"));
	return true;
}


bool RecordingMonitor::noticeRecordingStopped()
{
	if (m_showHideOnStartStop)
		hide();
	m_btnStartStop->setText(i18n("&Start"));
	return true;
}


bool RecordingMonitor::noticeRecordingConfigChanged(const RecordingConfig &c)
{
	m_labelRate->setText(QString().setNum(c.rate) + " kHz");
	return true;
}


bool RecordingMonitor::noticeRecordingContextChanged(const RecordingContext &c)
{
	switch (c.state()) {
	case RecordingContext::rsInvalid:
		m_labelStatus->setText(i18n("not running"));
		m_labelTime->setText(QString::null);
		m_labelSize->setText(QString::null);
		m_labelFileName->setText(QString::null);
		return true;
	case RecordingContext::rsRunning:
		m_labelStatus->setText(i18n("recording"));
		break;
	case RecordingContext::rsMonitor:
		m_labelStatus->setText(i18n("monitoring"));
		m_labelTime->setText(QString::null);
		m_labelSize->setText(QString::null);
		m_labelFileName->setText(QString::null);
		return true;
	case RecordingContext::rsError:
		m_labelStatus->setText(i18n("error occurred"));
		break;
	case RecordingContext::rsFinished:
		m_labelStatus->setText(i18n("finished"));
		break;
	}

	m_labelFileName->setText(c.outputFile());
	
	double s = c.outputTime();
	int m = (int)(s / 60);   s -= 60 * m;
	int h = m / 60;   m %= 60;
	int d = h / 24;   h %= 24;
	QString time;
	if (d) {
		time.sprintf("%dd - %02d:%02d:%05.2f", d, h, m, s);
	} else {
		time.sprintf("%02d:%02d:%05.2f", h, m, s);
	}		
	m_labelTime->setText(time);

    double B  = c.outputSize();
    double kB = B / 1024;
    double MB = kB / 1024;
    double GB = MB / 1024;
    QString size;
    size.sprintf("%.0f Byte", B);
    if (kB > 1) size.sprintf("%.3f kB", kB);
    if (MB > 1) size.sprintf("%.3f MB", MB);
    if (GB > 1) size.sprintf("%.3f GB", GB);	
	m_labelSize->setText(size);

	return true;
}



void RecordingMonitor::show()
{
	QWidget::show();
    WidgetPluginBase::show();
}


void RecordingMonitor::hide()
{
    WidgetPluginBase::hide();
    QWidget::hide();
}


void RecordingMonitor::showEvent(QShowEvent *e)
{
	QWidget::showEvent(e);
	WidgetPluginBase::showEvent(e);
}


void RecordingMonitor::hideEvent(QHideEvent *e)
{
	QWidget::hideEvent(e);
	WidgetPluginBase::hideEvent(e);
}


void RecordingMonitor::slotStartStopRecording()
{
	if (queryIsRecording()) {
		sendStopRecording();
	} else {
		sendStartRecording();
	}
}


void RecordingMonitor::slotHideShowToggle(bool on)
{
	m_showHideOnStartStop = on;
}
