/***************************************************************************
                          recording-monitor.h  -  description
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

#ifndef KRADIO_RECORDING_MONITOR_H
#define KRADIO_RECORDING_MONITOR_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qwidget.h>

#include "widgetplugins.h"
#include "recording-interfaces.h"


class QLabel;
class QPushButton;
class QCheckBox;

class RecordingMonitor : public QWidget,
				         public WidgetPluginBase,
                         public IRecordingClient
{
Q_OBJECT
public:

	RecordingMonitor(QWidget *parent, const QString &name);
	virtual ~RecordingMonitor();

	const QString &name() const { return PluginBase::name(); }
	      QString &name()       { return PluginBase::name(); }

	// WidgetPluginBase

public:
	virtual void   saveState (KConfig *) const;
	virtual void   restoreState (KConfig *);

	virtual bool   connect(Interface *i);
	virtual bool   disconnect(Interface *i);

	virtual ConfigPageInfo  createConfigurationPage();
	virtual QWidget        *createAboutPage();

	// IRecordingClient

RECEIVERS:
	bool noticeRecordingStarted();
	bool noticeRecordingStopped();
	bool noticeRecordingConfigChanged(const RecordingConfig &);
	bool noticeRecordingContextChanged(const RecordingContext &);

public slots:

	void    toggleShown() { WidgetPluginBase::toggleShown(); }
	void    show();
	void    hide();

    void    slotStartStopRecording();
    void    slotHideShowToggle(bool on);
    
protected:
	virtual void showEvent(QShowEvent *);
	virtual void hideEvent(QHideEvent *);

	const QWidget *getWidget() const { return this; }
	      QWidget *getWidget()       { return this; }


protected:

	bool         m_showHideOnStartStop;

	QLabel      *m_labelSize;
	QLabel      *m_labelTime;
	QLabel      *m_labelRate;
	QLabel      *m_labelFileName;
	QLabel      *m_labelStatus;
	QPushButton *m_btnStartStop;
	QCheckBox   *m_showHide;
};




#endif
