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

#include "radio_interfaces.h"
#include "radiodevicepool_interface.h"
#include "plugins.h"
#include "radioview_element.h"

class QWidgetStack;
class QToolButton;
class KComboBox;
                    

class RadioView : public QWidget,
				  public PluginBase,
                  public IRadioClient,
                  public IRadioDevicePoolClient
{
Q_OBJECT
public:

	RadioView(QWidget *parent, const QString &name);
	virtual ~RadioView();

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

	// PluginBase

public:
	virtual void   saveState (KConfig *) const;
	virtual void   restoreState (KConfig *);

	virtual bool   connect(Interface *i);
	virtual bool   disconnect(Interface *i);

	virtual void   noticeWidgetPluginShown(WidgetPluginBase *p, bool shown);
	
	virtual ConfigPageInfo  createConfigurationPage();
	virtual QWidget        *createAboutPage();


/* Ideas for the seeker:

   universal seeker:  just a seekleft+seekright button. right aligned under normal buttons.
                      this increases the main gui's height.
   frequency seeker:  gui as in current implementation



   Some other ideas:

   RadioView manages some GuiControls with a common interface. Calls
   GuiControl->(dis)connect for each ActiveDevice change. Turns GuiControl
   on/off, depending on return value of connect call.

*/

protected slots:

    void slotPower (bool on);
    void slotConfigure (bool show);
    void slotComboStationSelected(int);


protected:
	QToolButton          *btnPower;
	QToolButton          *btnConfigure;
	QToolButton          *btnQuit;
	QToolButton          *btnQuickbar;
	KComboBox            *comboStations;

	typedef  QPtrList<RadioViewElement>         ElementList;
	typedef  QPtrListIterator<RadioViewElement> ElementListIterator;

	ElementList           elements;
	QWidgetStack *        widgetStacks[clsClassMAX];
	float                 maxUsability[clsClassMAX];

	IRadioDevice          *currentDevice;
};




#endif
