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

QuickBar::QuickBar(QWidget * parent, const char * name)
  : QWidget(parent, name),
    PluginBase(name),

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

	if (a) kdDebug() << "QuickBar: IRadioClient connected\n";
	
	return a;
}


bool QuickBar::disconnect(Interface *i)
{
	bool a = IRadioClient::disconnect(i);

	if (a) kdDebug() << "QuickBar: IRadioClient disconnected\n";

	return a;
}


// PluginBase methods


void QuickBar::restoreState (KConfig *config)
{
    config->setGroup(QString("quickBar-") + name());

	m_saveDesktop  = config->readNumEntry ("desktop", 1);
	m_saveSticky   = config->readBoolEntry("sticky",  false);
	m_saveGeometry = config->readRectEntry("Geometry");

    if (config->readBoolEntry("hidden", false))
        hide();
    else
        show();

	int nStations = config->readNumEntry("nStations", 0);
	for (int i = 1; i <= nStations; ++i) {
		QString s = config->readEntry(QString("stationID-") + QString().setNum(i), "");
		if (s.length())
			m_stationIDs += s;
	}

	rebuildGUI();
}


void QuickBar::saveState (KConfig *config) const
{
    config->setGroup(QString("quickBar-") + name());

	getKWinState();

    config->writeEntry("hidden", isHidden());

	config->writeEntry("sticky",   m_saveSticky);
	config->writeEntry("desktop",  m_saveDesktop);
	config->writeEntry("Geometry", m_saveGeometry);

	config->writeEntry("nStations", m_stationIDs.size());
	int i = 1;
	for (QStringList::const_iterator it = m_stationIDs.begin(); it != m_stationIDs.end(); ++it, ++i) {
		config->writeEntry(QString("stationID-") + QString().setNum(i), *it);
	}
}


QFrame *QuickBar::internal_createConfigurationPage(KDialogBase */*dlg*/)
{
	// FIXME
	return NULL;
}


QFrame *QuickBar::internal_createAboutPage(QWidget */*parent*/)
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
		// FIXME: please test, is this here ok?
		m_buttonGroup->setButton(-1);
	}

	setCaption(pwr && rs.isValid() ? rs.name() : "KRadio");
}



// KDE/Qt gui


void QuickBar::getKWinState() const
{
	if (isVisible()) {
		KWin::Info  i = KWin::info(winId());
		m_saveSticky    = i.onAllDesktops;
		m_saveDesktop   = i.desktop;
		m_saveGeometry  = geometry();
	}
}


void QuickBar::rebuildGUI()
{
	if (m_layout) delete m_layout;
	m_layout = new ButtonFlowLayout(this);

	m_layout->setMargin(1);
	m_layout->setSpacing(2);

	if (m_buttonGroup) delete m_buttonGroup;
	m_buttonGroup = new QButtonGroup(this);
    QObject::connect (m_buttonGroup, SIGNAL(clicked(int)), this, SLOT(buttonClicked(int)));
	
	// we use buttonGroup to enable automatic toggle/untoggle
	m_buttonGroup->setExclusive(true);
	m_buttonGroup->setFrameStyle(QFrame::NoFrame);

    int buttonID = 0;
    const RawStationList &stations = queryStations().all();

    for (QStringList::iterator it = m_stationIDs.begin(); it != m_stationIDs.end(); ++it, ++buttonID) {

		const RadioStation &rs = stations.stationWithID(*it);
		if (! rs.isValid()) continue;

        KToolBarButton *b = new KToolBarButton(QPixmap(rs.iconName()), buttonID,
                                               this, "",
                                               m_showShortName ? rs.shortName() : rs.name());
	    b->setToggleButton(true);

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




void QuickBar::setOn(bool on)
{
	if (on && !isVisible())
		show();
	else if (!on && isVisible())
		hide();
}


void QuickBar::show()
{
	bool wasHidden = !isVisible();

	QWidget::show();

    if (wasHidden) {
     	KWin::setOnAllDesktops(winId(), m_saveSticky);
    	KWin::setType(winId(), NET::Toolbar);

    	setGeometry(m_saveGeometry);
    }
}


void QuickBar::hide()
{
    getKWinState();
    QWidget::hide();
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



