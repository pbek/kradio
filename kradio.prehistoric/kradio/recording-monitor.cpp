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
#include "recording-datamonitor.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <kconfig.h>

#include <kaboutdata.h>
#include "aboutwidget.h"

RecordingMonitor::RecordingMonitor(QWidget *parent, const QString &name)
  : QWidget(parent, (const char*)name),
    WidgetPluginBase(name, i18n("Recording Monitor")),
    m_showHideOnStartStop(true)
{
	setCaption(i18n("Recording Monitor"));

	QVBoxLayout *l = new QVBoxLayout(this, 10, 4);
	QGridLayout *l0 = new QGridLayout(l, 5, 3);

	l0->addWidget(                  new QLabel(i18n("Status"),         this), 0, 0);
	l0->addWidget(m_labelStatus   = new QLabel(i18n("<undefined>"),    this), 0, 1);
	l0->addWidget(                  new QLabel(i18n("Recording File"), this), 1, 0);
	l0->addWidget(m_labelFileName = new QLabel(i18n("<undefined>"),    this), 1, 1);
	l0->addWidget(                  new QLabel(i18n("File Size"),      this), 2, 0);
	l0->addWidget(m_labelSize     = new QLabel(i18n("<undefined>"),    this), 2, 1);
	l0->addWidget(                  new QLabel(i18n("Recording Time"), this), 3, 0);
	l0->addWidget(m_labelTime     = new QLabel(i18n("<undefined>"),    this), 3, 1);
	l0->addWidget(                  new QLabel(i18n("Sample Rate"),    this), 4, 0);
	l0->addWidget(m_labelRate     = new QLabel(i18n("<undefined>"),    this), 4, 1);

	l0->addItem(new QSpacerItem(3,1), 0, 2);

	QPushButton *close     = new QPushButton(i18n("&Close"), this);
	m_btnStartStop         = new QPushButton(i18n("&Start"), this);
	QObject::connect(close, SIGNAL(clicked()), this, SLOT(hide()));
	QObject::connect(m_btnStartStop, SIGNAL(clicked()), this, SLOT(slotStartStopRecording()));

	m_showHide = new QCheckBox(i18n("Hide/Show when recording stops/starts"), this);
	QObject::connect(m_showHide, SIGNAL(toggled(bool)), this, SLOT(slotHideShowToggle(bool)));
	m_showHide->setChecked(m_showHideOnStartStop);

	m_btnMonitor = new QCheckBox(i18n("Monitor Input"), this);
	QObject::connect(m_btnMonitor, SIGNAL(toggled(bool)), this, SLOT(slotEnableMonitor(bool)));
	m_showHide->setChecked(queryIsMonitoring());
	m_showHide->setDisabled(queryIsRecording());

	m_dataMonitor = new RecordingDataMonitor(this, NULL);
	
	QHBoxLayout *hl0 = new QHBoxLayout(l);	
	hl0->addWidget(m_dataMonitor);

	QHBoxLayout *hl1 = new QHBoxLayout(l);
	hl1->addItem(new QSpacerItem(10, 1));
	hl1->addWidget(m_showHide);
	hl1->addWidget(m_btnMonitor);
	hl1->addItem(new QSpacerItem(10, 1));

	QHBoxLayout *hl2 = new QHBoxLayout(l);
	hl2->addItem(new QSpacerItem(10, 1));
	hl2->addWidget(close);
	hl2->addWidget(m_btnStartStop);
	hl2->addItem(new QSpacerItem(10, 1));
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

	WidgetPluginBase::restoreState(config, false);
	m_showHideOnStartStop = config->readBoolEntry("showHideOnStartStop", true);
	m_showHide->setChecked(m_showHideOnStartStop);
}


bool   RecordingMonitor::connect(Interface *i)
{
	bool a = IRecordingClient::connect(i);
	bool b = m_dataMonitor->connect(i);
	return a || b;
}

bool   RecordingMonitor::disconnect(Interface *i)
{
	bool a = IRecordingClient::disconnect(i);
	bool b = m_dataMonitor->disconnect(i);
	return a || b;
}

ConfigPageInfo  RecordingMonitor::createConfigurationPage()
{
	return ConfigPageInfo();
}

AboutPageInfo   RecordingMonitor::createAboutPage()
{
    KAboutData aboutData("kradio",
						 NULL,
                         NULL,
                         I18N_NOOP("Recording Monitor Plugin for KRadio"),
                         KAboutData::License_GPL,
                         "(c) 2002, 2003 Martin Witte",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  "", "witte@kawo1.rwth-aachen.de");

	return AboutPageInfo(
	          new KRadioAboutWidget(aboutData, KRadioAboutWidget::AbtTabbed),
	          i18n("Recording Monitor"),
	          i18n("Recording Monitor Plugin"),
	          "goto"
		   );
}


// IRecordingClient

bool RecordingMonitor::noticeRecordingStarted()
{
	if (m_showHideOnStartStop)
		show();
	m_btnStartStop->setText(i18n("&Stop"));
	m_btnMonitor->setDisabled(true);
	m_btnMonitor->blockSignals(true);
	m_btnMonitor->setChecked(false);
	m_btnMonitor->blockSignals(false);
	return true;
}


bool RecordingMonitor::noticeMonitoringStarted()
{
	m_btnMonitor->blockSignals(true);
	m_btnMonitor->setChecked(true);
	m_btnMonitor->blockSignals(false);
	return true;
}


bool RecordingMonitor::noticeRecordingStopped()
{
	if (m_showHideOnStartStop)
		hide();
	m_btnStartStop->setText(i18n("&Start"));
	m_btnMonitor->setDisabled(false);
	m_btnMonitor->blockSignals(true);
	m_btnMonitor->setChecked(false);
	m_btnMonitor->blockSignals(false);
	return true;
}


bool RecordingMonitor::noticeMonitoringStopped()
{
	m_btnMonitor->blockSignals(true);
	m_btnMonitor->setChecked(false);
	m_btnMonitor->blockSignals(false);
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


void RecordingMonitor::slotEnableMonitor(bool on)
{
	if (on)
		sendStartMonitoring();
	else
		sendStopMonitoring();
}
