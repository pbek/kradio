/***************************************************************************
                          radioview.h  -  description
                             -------------------
    begin                : Mit Mai 28 2003
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

#ifndef KRADIO_RADIOVIEW_H
#define KRADIO_RADIOVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qobjectlist.h>

#include "radio_interfaces.h"
#include "radiodevicepool_interface.h"
#include "widgetplugins.h"
#include "radioview_element.h"
#include "recording-interfaces.h"

class QWidgetStack;
class QToolButton;
class KComboBox;
class QTabWidget;





class RadioView : public QWidget,
				  public WidgetPluginBase,
                  public IRadioClient,
                  public IRadioDevicePoolClient,
                  public IRecordingClient
{
Q_OBJECT
public:

	RadioView(QWidget *parent, const QString &name);
	virtual ~RadioView();

	const QString &name() const { return PluginBase::name(); }
	      QString &name()       { return PluginBase::name(); }

public slots:
	// connects destroy-msg with remove-function
	bool addElement    (RadioViewElement *);
	bool removeElement (QObject *);
	
protected:
	void selectTopWidgets();

	
    // IRadioClient

RECEIVERS:
	bool noticePowerChanged(bool on);
	bool noticeStationChanged (const RadioStation &, int idx);
	bool noticeStationsChanged(const StationList &sl);

	// IRadioDevicePoolClient

RECEIVERS:
	bool noticeActiveDeviceChanged(IRadioDevice *rd);
	bool noticeDevicesChanged(const QPtrList<IRadioDevice> &)  { return false; }
	bool noticeDeviceDescriptionChanged(const QString &) { return false; }

	// IRecordingClient
	
RECEIVERS:
	bool noticeRecordingStarted();
	bool noticeRecordingStopped();
	bool noticeRecordingConfigChanged(const RecordingConfig &)   { return false; }
	bool noticeRecordingContextChanged(const RecordingContext &) { return false; }

	
	// WidgetPluginBase

public:
	virtual void   saveState (KConfig *) const;
	virtual void   restoreState (KConfig *);

	virtual bool   connect(Interface *i);
	virtual bool   disconnect(Interface *i);

	virtual void   noticeWidgetPluginShown(WidgetPluginBase *p, bool shown);
	
	virtual ConfigPageInfo  createConfigurationPage();
	virtual QWidget        *createAboutPage();

protected slots:

    void slotPower (bool on);
    void slotConfigure (bool show);
    void slotRecord (bool start);
    void slotComboStationSelected(int);

    void slotConfigPageDeleted(QObject*);
    void slotElementConfigPageDeleted(QObject*);

public slots:

	void    toggleShown() { WidgetPluginBase::toggleShown(); }
	void    show();
	void    hide();

protected:
	virtual void showEvent(QShowEvent *);
	virtual void hideEvent(QHideEvent *);

	const QWidget *getWidget() const { return this; }
	      QWidget *getWidget()       { return this; }

    void    addConfigurationTabFor(RadioViewElement *, QTabWidget *);

    
protected:
	QToolButton          *btnPower;
	QToolButton          *btnConfigure;
	QToolButton          *btnQuit;
	QToolButton          *btnRecording;
	KComboBox            *comboStations;

	struct ElementCfg
	{
		RadioViewElement *element;
		QObject          *cfg;		
		ElementCfg()                                : element(NULL), cfg(NULL) {}
		ElementCfg(RadioViewElement *e, QObject *w) : element(e), cfg(w) {}
		ElementCfg(RadioViewElement *e)             : element(e), cfg(NULL) {}
		ElementCfg(QObject *w)                      : element(NULL), cfg(w) {}
		bool operator == (const ElementCfg &x) const;
	};
	
	typedef  QPtrList<RadioViewElement>         ElementList;
	typedef  QPtrListIterator<RadioViewElement> ElementListIterator;
    typedef  QValueList<ElementCfg>             ElementCfgList;
    typedef  QValueListIterator<ElementCfg>     ElementCfgListIterator;

	ElementList           elements;
	ElementCfgList        elementConfigPages;
	QObjectList           configPages;
	QWidgetStack *        widgetStacks[clsClassMAX];
	float                 maxUsability[clsClassMAX];

	IRadioDevice          *currentDevice;
};




#endif
