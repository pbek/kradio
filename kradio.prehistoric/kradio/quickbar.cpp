/***************************************************************************
                          quickbar.cpp  -  description
                             -------------------
    begin                : Mon Feb 11 2002
    copyright            : (C) 2002 by Martin Witte / Frank Schwanz
    email                : witte@kawo1.rwth-aachen.de / schwanz@fh-brandenburg.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qtooltip.h>
#include <qnamespace.h>
#include <qhbuttongroup.h>
#include <qvbuttongroup.h>
#include <ktoolbarbutton.h>

#include <kwin.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>

#include "buttonflowlayout.h"
#include "quickbar.h"
#include "stationlist.h"
#include "radiostation.h"
#include "quickbar-configuration.h"

QuickBar::QuickBar(QWidget * parent, const char * name)
  : QWidget(parent, name),
    WidgetPluginBase(name, "Quickbar Plugin"),
    m_layout(NULL),
    m_buttonGroup(NULL),
    m_showShortName(true)
{
}


QuickBar::~QuickBar()
{
}


bool QuickBar::connect(Interface *i)
{
	bool a = IRadioClient::connect(i);
	bool b = IStationSelection::connect(i);

//	if (a) kdDebug() << "QuickBar: IRadioClient connected\n";
	
	return a || b;
}


bool QuickBar::disconnect(Interface *i)
{
	bool a = IRadioClient::disconnect(i);
	bool b = IStationSelection::disconnect(i);

//	if (a) kdDebug() << "QuickBar: IRadioClient disconnected\n";

	return a || b;
}


// IStationSelection

bool QuickBar::setStationSelection(const QStringList &sl)
{
	m_stationIDs = sl;
	rebuildGUI();
	notifyStationSelectionChanged(m_stationIDs);
	return true;
}

// PluginBase methods


void QuickBar::restoreState (KConfig *config)
{
    config->setGroup(QString("quickBar-") + name());

	WidgetPluginBase::restoreState(config);

	int nStations = config->readNumEntry("nStations", 0);
	for (int i = 1; i <= nStations; ++i) {
		QString s = config->readEntry(QString("stationID-") + QString().setNum(i), QString::null);
		if (s.length())
			m_stationIDs += s;
	}

	rebuildGUI();
	notifyStationSelectionChanged(m_stationIDs);
}


void QuickBar::saveState (KConfig *config) const
{
    config->setGroup(QString("quickBar-") + name());

	WidgetPluginBase::saveState(config);

	config->writeEntry("nStations", m_stationIDs.size());
	int i = 1;
	for (QStringList::const_iterator it = m_stationIDs.begin(); it != m_stationIDs.end(); ++it, ++i) {
		config->writeEntry(QString("stationID-") + QString().setNum(i), *it);
	}
}


ConfigPageInfo QuickBar::createConfigurationPage()
{
	QuickbarConfiguration *conf = new QuickbarConfiguration(NULL);
	connect (conf);
	return ConfigPageInfo(
		conf,
		"Quickbar",
		"Quickbar Configuration",
		"view_icon"
	);
}


QWidget *QuickBar::createAboutPage()
{
	// FIXME
	return NULL;
}


// IRadio methods

bool QuickBar::noticePowerChanged(bool /*on*/)
{
	activateCurrentButton();
	return true;
}


bool QuickBar::noticeStationChanged (const RadioStation &rs, int /*idx*/)
{
	activateButton(rs);
	return true;
}


bool QuickBar::noticeStationsChanged(const StationList &/*sl*/)
{
	// FIXME
	// we can remove no longer existent stationIDs,
	// but it doesn't matter if we don't care.
	rebuildGUI();
	return true;
}


// button management methods

void QuickBar::buttonClicked(int id)
{
	// ouch, but we are still using QStringList :(
	int k = 0;
	for (QStringList::iterator it = m_stationIDs.begin(); it != m_stationIDs.end(); ++it, ++k) {
		if (k == id) {
			const RawStationList &sl = queryStations().all();
			const RadioStation &rs = sl.stationWithID(*it);
			sendActivateStation(rs);
		}
	}
	// Problem: if we click a button twice, there will be no
	// "station changed"-notification. Thus it would be possible to
	// enable a button even if power is off or the radio does not
	// accept the radiostation
	activateCurrentButton();
}


int QuickBar::getButtonID(const RadioStation &rs) const
{
	QString stationID = rs.stationID();
	int k = 0;
	for (QStringList::const_iterator it = m_stationIDs.begin(); it != m_stationIDs.end(); ++it, ++k) {
		if (*it == stationID)
			return k;
	}
	return -1;
}


void QuickBar::activateCurrentButton()
{
	activateButton(queryCurrentStation());
}


void QuickBar::activateButton(const RadioStation &rs)
{
	int buttonID = getButtonID(rs);
	bool pwr = queryIsPowerOn();
	
	if (pwr && buttonID >= 0) {
		m_buttonGroup->setButton(buttonID);
	} else {
		for (QToolButton *b = m_buttons.first(); b; b = m_buttons.next()) {
			b->setOn(false);
		}
	}
	setCaption(pwr && rs.isValid() ? rs.longName() : QString("KRadio"));
}



// KDE/Qt gui


void QuickBar::rebuildGUI()
{
	if (m_layout) delete m_layout;
	if (m_buttonGroup) delete m_buttonGroup;

	for (QPtrListIterator<QToolButton> it(m_buttons); it.current(); ++it)
		delete it.current();
   	m_buttons.clear();

	m_layout = new ButtonFlowLayout(this);
	m_layout->setMargin(1);
	m_layout->setSpacing(2);
	
	m_buttonGroup = new QButtonGroup(this);
    QObject::connect (m_buttonGroup, SIGNAL(clicked(int)), this, SLOT(buttonClicked(int)));
	// we use buttonGroup to enable automatic toggle/untoggle
	m_buttonGroup->setExclusive(true);
	m_buttonGroup->setFrameStyle(QFrame::NoFrame);
	m_buttonGroup->show();

    int buttonID = 0;
    const RawStationList &stations = queryStations().all();

    for (QStringList::iterator it = m_stationIDs.begin(); it != m_stationIDs.end(); ++it, ++buttonID) {

		const RadioStation &rs = stations.stationWithID(*it);
		if (! rs.isValid()) continue;

        QToolButton *b = new QToolButton(this, QString::null);
        m_buttons.append(b);
	    b->setToggleButton(true);
	    if (rs.iconName())
			b->setIconSet(QPixmap(rs.iconName()));
		else
			b->setText(m_showShortName ? rs.shortName() : rs.name());

        b->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

        QToolTip::add(b, rs.longName());
        if (isVisible()) b->show();

        
        m_buttonGroup->insert(b, buttonID);
        m_layout->add(b);
    }
    
    // activate correct button
    activateCurrentButton();

    // calculate geometry
    if (m_layout) {
		QRect r = geometry();
		int h = m_layout->heightForWidth( r.width());

		if (h > r.height())
			setGeometry(r.x(), r.y(), r.width(), h);
    }
}




void QuickBar::show()
{
  	KWin::setType(winId(), NET::Toolbar);
    WidgetPluginBase::show();
	QWidget::show();
    WidgetPluginBase::show();
}

void QuickBar::hide()
{
    WidgetPluginBase::hide();
    QWidget::hide();
}

void    QuickBar::showEvent(QShowEvent *e)
{
	QWidget::showEvent(e);
	WidgetPluginBase::showEvent(e);
}

void    QuickBar::hideEvent(QHideEvent *e)
{
	QWidget::hideEvent(e);
	WidgetPluginBase::hideEvent(e);
}


void QuickBar::setGeometry (int x, int y, int w, int h)
{
	if (m_layout) {
		QSize marginSize(m_layout->margin()*2, m_layout->margin()*2);
		setMinimumSize(m_layout->minimumSize(QSize(w, h) - marginSize) + marginSize);
	}
	QWidget::setGeometry (x, y, w, h);
}


void QuickBar::setGeometry (const QRect &r)
{
	setGeometry (r.x(), r.y(), r.width(), r.height());
}


void QuickBar::resizeEvent (QResizeEvent *e)
{
	// minimumSize might change because of the flow layout
	if (m_layout) {
		QSize marginSize(m_layout->margin()*2, m_layout->margin()*2);
		setMinimumSize(m_layout->minimumSize(e->size() - marginSize) + marginSize);
	}

	QWidget::resizeEvent (e);
}


