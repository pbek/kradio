/***************************************************************************
                          radioview.cpp  -  description
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

#include <qwidgetstack.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qslider.h>
#include <qfile.h>

#include <kcombobox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kapplication.h>
#include <kwin.h>
#include <kconfig.h>

#include "radioview.h"
#include "radiodevice_interfaces.h"
#include "stationlist.h"
#include "radiostation.h"
#include "pluginmanager.h"
#include "plugin_configuration_dialog.h"


#include "radioview_frequencyradio.h"
#include "radioview_volume.h"
#include "radioview_frequencyseeker.h"

#include "radioview-configuration.h"

///////////////////////////////////////////////////////////////////////

bool RadioView::ElementCfg::operator == (const ElementCfg &x) const
{
	if (!x.element || !element)
		return x.cfg == cfg;
	if (!x.cfg || !cfg)
		return x.element == element;
	return element == x.element && cfg == x.cfg;
}

///////////////////////////////////////////////////////////////////////

RadioView::RadioView(QWidget *parent, const QString &name)
  : QWidget(parent, (const char*)name),
    WidgetPluginBase(name),
    currentDevice(NULL)
{
	for (int i = 0; i < clsClassMAX; ++i)
		maxUsability[i] = 0;

	QBoxLayout *l01 = new QBoxLayout(this, QBoxLayout::LeftToRight, /*spacing=*/3);
	l01->setMargin(2);
	widgetStacks[clsRadioSound] = new QWidgetStack (this);
    l01->addWidget(widgetStacks[clsRadioSound]);

	QBoxLayout *l02 = new QBoxLayout(l01, QBoxLayout::Down);
	QBoxLayout *l03 = new QBoxLayout(l02, QBoxLayout::LeftToRight);
    widgetStacks[clsRadioSeek] = new QWidgetStack (this);
    comboStations = new KComboBox (this);
    l02->addWidget (widgetStacks[clsRadioSeek]);
    l02->addWidget (comboStations);

	widgetStacks[clsRadioDisplay] = new QWidgetStack (this);
	l03->addWidget(widgetStacks[clsRadioDisplay]);

	QGridLayout *l04 = new QGridLayout (l03, /*rows=*/ 2, /*cols=*/ 2);
	btnPower         = new QToolButton(this);
	btnPower->setToggleButton(true);
	btnQuickbar      = new QToolButton(this);
	btnConfigure     = new QToolButton(this);
	btnConfigure->setToggleButton(true);
	btnQuit          = new QToolButton(this);
	l04->addWidget (btnPower,     0, 0);
	l04->addWidget (btnQuickbar,  0, 1);
	l04->addWidget (btnConfigure, 1, 0);
	l04->addWidget (btnQuit,      1, 1);

	btnPower->setIconSet(SmallIconSet("kradio_muteoff"));
	btnQuickbar->setIconSet(SmallIconSet("1uparrow"));
	btnConfigure->setIconSet(SmallIconSet("configure"));
	btnQuit->setIconSet(SmallIconSet("exit"));

    widgetStacks[clsRadioSound]  ->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,   QSizePolicy::Preferred));
    widgetStacks[clsRadioDisplay]->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
    widgetStacks[clsRadioSeek]   ->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    comboStations                ->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    comboStations->setMinimumHeight(28);
    
    
	QObject::connect(btnPower,      SIGNAL(toggled(bool)),
                     this,          SLOT(slotPower(bool)));
	QObject::connect(btnQuit,       SIGNAL(clicked()),
                     kapp,          SLOT(quit()));
	QObject::connect(btnConfigure,  SIGNAL(toggled(bool)),
                     this,          SLOT(slotConfigure(bool)));
    QObject::connect(comboStations, SIGNAL(activated(int)),
	                 this,          SLOT(slotComboStationSelected(int)));


    // testing
    addElement (new RadioViewFrequencyRadio (this, QString::null));
    addElement (new RadioViewVolume(this, QString::null));
    addElement (new RadioViewFrequencySeeker(this, QString::null));
}


RadioView::~RadioView ()
{
	QPtrListIterator<QObject> it(configPages);
	while (configPages.first()) {
		delete configPages.first();
	}
	configPages.clear();
}


bool RadioView::addElement (RadioViewElement *e)
{
	if (!e) return false;

	RadioViewClass cls = e->getClass();
	
	if (cls < 0 || cls >= clsClassMAX)
		return false;


	e->reparent(this, QPoint(0, 0), true);
	QObject::connect(e,    SIGNAL(destroyed(QObject*)),
	                 this, SLOT(removeElement(QObject*)));
	elements.append(e);
	widgetStacks[cls]->addWidget(e);

	// connect Element with device, disconnect doesn't matter (comp. removeElement)
	// other devices follow if currentDevice changes
	e->connect(currentDevice);

	QPtrListIterator<QObject> it(configPages);
	for (; it.current(); ++it) {
		addConfigurationTabFor(e, (QTabWidget *)it.current());
	}

	selectTopWidgets();
	
	return true;
}


bool RadioView::removeElement (QObject *_e)
{
	RadioViewElement *e = dynamic_cast<RadioViewElement*>(_e);
    if (!e)
		return false;

	ElementCfgListIterator it;
	while ((it = elementConfigPages.find(e)) != elementConfigPages.end()) {
		delete (*it).cfg;
		// it must not used behind, the element will be deleted automatically
		// by slotElementConfigPageDeleted
	}

    e->disconnect(currentDevice);
	RadioViewClass cls = e->getClass();
	QObject::disconnect(e,    SIGNAL(destroyed(QObject*)),
	                    this, SLOT(removeElement(QObject*)));
	widgetStacks[cls]->removeWidget(e);
	elements.remove(e);
	
	selectTopWidgets();
	
	return true;
}


void RadioView::selectTopWidgets()
{
	for (int i = 0; i < clsClassMAX; ++i)
		maxUsability[i] = 0;
		
	for (ElementListIterator i(elements); i.current(); ++i) {
		RadioViewElement *e  = i.current();
		RadioViewClass   cls = e->getClass();
		float u = e->getUsability(currentDevice);
		if (u > maxUsability[cls]) {
			maxUsability[cls] = u;
			widgetStacks[cls]->raiseWidget(e);
		}
	}
	// adjustLayout!?
}


// IRadioClient

bool RadioView::noticePowerChanged(bool on)
{
	btnPower->setIconSet(SmallIconSet( on ? "kradio_muteon" : "kradio_muteoff"));
	btnPower->setOn(on);
	return true;
}


bool RadioView::noticeStationChanged (const RadioStation &, int idx)
{
	// add 1 for "no preset defined" entry
	comboStations->setCurrentItem(idx + 1);
	return true;
}


bool RadioView::noticeStationsChanged(const StationList &sl)
{
	const RawStationList &list = sl.all();

    comboStations->clear();
    comboStations->insertItem("<" + i18n("no preset defined") + ">");
	
    for (RawStationList::Iterator i(list); i.current(); ++i) {
		RadioStation *stn = i.current();
		QString icon = stn->iconName();		
		if (icon.length() && QFile(icon).exists()) {
			comboStations->insertItem(QPixmap(icon));
        } else {
            comboStations->insertItem(stn->name());
        }
    }

    noticeStationChanged(queryCurrentStation(), queryCurrentStationIdx());
	return true;
}

// IRadioDevicePoolClient

bool RadioView::noticeActiveDeviceChanged(IRadioDevice *newDevice)
{
	IRadioDevice *oldDevice = currentDevice;
	currentDevice = newDevice;
	
	for (ElementListIterator i(elements); i.current(); ++i) {
		RadioViewElement *e = i.current();
		e->disconnect(oldDevice);
		e->connect(currentDevice);
	}
	
	selectTopWidgets();
	return true;
}


// Interface

bool RadioView::connect(Interface *i)
{
	bool a = IRadioClient::connect(i);
	bool b = IRadioDevicePoolClient::connect(i);

	return a || b;
}


bool RadioView::disconnect(Interface *i)
{
	bool a = IRadioClient::disconnect(i);
	bool b = IRadioDevicePoolClient::disconnect(i);

	return a || b;
}


// WidgetPluginBase

void   RadioView::saveState (KConfig *config) const
{
    config->setGroup(QString("radioview-") + name());

	getKWinState();

    config->writeEntry("hidden", isHidden());

	config->writeEntry("sticky",   m_saveSticky);
	config->writeEntry("desktop",  m_saveDesktop);
	config->writeEntry("geometry", m_saveGeometry);

	for (ElementListIterator i(elements); i.current(); ++i) {
		RadioViewElement *e  = i.current();
		e->saveState(config);
	}
}


void   RadioView::restoreState (KConfig *config)
{
	config->setGroup(QString("radioview-") + name());

	m_saveDesktop  = config->readNumEntry ("desktop", 1);
	m_saveSticky   = config->readBoolEntry("sticky",  false);
	m_saveGeometry = config->readRectEntry("geometry");

    if (config->readBoolEntry("hidden", false))
        hide();
    else
        show();


	for (ElementListIterator i(elements); i.current(); ++i) {
		RadioViewElement *e  = i.current();
		e->restoreState(config);
	}
}


ConfigPageInfo RadioView::createConfigurationPage()
{
	RadioViewConfiguration *c = new RadioViewConfiguration();

	for (ElementListIterator i(elements); i.current(); ++i) {
		addConfigurationTabFor(i.current(), c);
	}

	configPages.append(c);
	QObject::connect(c,    SIGNAL(destroyed(QObject *)),
	                 this, SLOT(slotConfigPageDeleted(QObject *)));
	
	return ConfigPageInfo(
		c,
		"Display",
		"Display Configuration",
		"openterm"
	);
}


void RadioView::addConfigurationTabFor(RadioViewElement *e, QTabWidget *c)
{
	if (!e || !c)
		return;

	ConfigPageInfo inf = e->createConfigurationPage();

	if (inf.configPage) {

		if (inf.iconName.length()) {
			c->addTab(inf.configPage, QIconSet(SmallIconSet(inf.iconName)), inf.itemName);
		} else {
			c->addTab(inf.configPage, inf.itemName);
		}

		elementConfigPages.push_back(ElementCfg(e, inf.configPage));
		QObject::connect(inf.configPage, SIGNAL(destroyed(QObject *)),
						 this, SLOT(slotElementConfigPageDeleted(QObject *)));
	}
}


QWidget *RadioView::createAboutPage()
{
	return NULL; // FIXME
}


void RadioView::noticeWidgetPluginShown(WidgetPluginBase *p, bool shown)
{
	if (m_manager && (WidgetPluginBase*)m_manager->getConfigDialog() == p)
		btnConfigure->setOn(shown);
}


// own Stuff

void RadioView::slotPower(bool on)
{
	on ? sendPowerOn() : sendPowerOff();
	btnPower->setOn(queryIsPowerOn());
}


void RadioView::slotConfigure(bool b)
{
	QWidget *w = m_manager ? m_manager->getConfigDialog() : NULL;
	if (w) b ? w->show() : w->hide();
	if (!w)
		btnConfigure->setOn(false);
}


void RadioView::slotComboStationSelected(int idx)
{
	if (idx > 0) {
		sendActivateStation(idx - 1);
	} else {
		comboStations->setCurrentItem(queryCurrentStationIdx() + 1);		
	}
}


void RadioView::slotConfigPageDeleted(QObject *o)
{
	configPages.remove(o);
}


void RadioView::slotElementConfigPageDeleted(QObject *o)
{
	ElementCfgListIterator it;
	while ((it = elementConfigPages.find(o)) != elementConfigPages.end()) {
		elementConfigPages.remove(it);		
	}
}

void RadioView::toggleShown()
{
	if (isHidden())
		show();
	else
		hide();
}

void RadioView::show(bool on)
{
	if (on && isHidden())
		show();
	else if (!on && !isHidden())
		hide();
}

void RadioView::show()
{
	bool wasHidden = !isVisible();

	QWidget::show();

    if (wasHidden) {
     	KWin::setOnAllDesktops(winId(), m_saveSticky);
    	KWin::setType(winId(), NET::Toolbar);

    	setGeometry(m_saveGeometry);
    }
}


void RadioView::hide()
{
    getKWinState();
    QWidget::hide();
}


void RadioView::showEvent(QShowEvent *e)
{
	QWidget::showEvent(e);
	notifyManager(true);
}


void RadioView::hideEvent(QHideEvent *e)
{
	QWidget::hideEvent(e);
	notifyManager(false);
}


void RadioView::getKWinState() const
{
	if (isVisible()) {
		KWin::Info    i = KWin::info(winId());
		m_saveSticky    = i.onAllDesktops;
		m_saveDesktop   = i.desktop;
		m_saveGeometry  = geometry();
	}
}


