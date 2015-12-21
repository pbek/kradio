/***************************************************************************
                          quickbar.cpp  -  description
                             -------------------
    begin                : Mon Feb 11 2002
    copyright            : (C) 2002 by Martin Witte / Frank Schwanz
    email                : emw-kradio@nocabal.de / schwanz@fh-brandenburg.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtGui/QToolTip>
#include <QtGui/QToolButton>
#include <QtGui/QButtonGroup>
#include <QtGui/QResizeEvent>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
//#include <QtCore/QNameSpace>

//#include <ktoolbarbutton.h>
#include <kwindowinfo.h>
#include <kwindowsystem.h>
#include <klocalizedstring.h>
#include <kglobal.h>
#include <kconfiggroup.h>
#include <kaboutdata.h>
#include <kbuttongroup.h>
#include <kicon.h>

//#include "aboutwidget.h"
#include "station-drag-object.h"
#include "stationlist.h"
#include "radiostation.h"

#include "buttonflowlayout4.h"
#include "quickbar-configuration.h"
#include "quickbar.h"

///////////////////////////////////////////////////////////////////////
//// plugin library functions

PLUGIN_LIBRARY_FUNCTIONS(QuickBar, PROJECT_NAME, i18n("Radio Station Quick Selection Toolbar"));

/////////////////////////////////////////////////////////////////////////////

QuickBar::QuickBar(const QString &instanceID, const QString &name)
  : QWidget(),
    WidgetPluginBase(this, instanceID, name, i18n("Quickbar Plugin")),
    m_layout(NULL),
    m_showShortName(true),
    m_ignoreNoticeActivation(false)
{
    QObject::connect(&m_mapper, SIGNAL(mapped(const QString &)), this, SLOT(buttonToggled(const QString &)));
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    autoSetCaption();
    setAcceptDrops(true);
}


QuickBar::~QuickBar()
{
}


bool QuickBar::connectI(Interface *i)
{
    bool a = IRadioClient::connectI(i);
    bool b = IStationSelection::connectI(i);
    bool c = PluginBase::connectI(i);

    return a || b || c;
}


bool QuickBar::disconnectI(Interface *i)
{
    bool a = IRadioClient::disconnectI(i);
    bool b = IStationSelection::disconnectI(i);
    bool c = PluginBase::disconnectI(i);

    return a || b || c;
}


// IStationSelection

bool QuickBar::setStationSelection(const QStringList &sl)
{
    if (m_stationIDs != sl) {
        m_stationIDs = sl;
        rebuildGUI();
        notifyStationSelectionChanged(m_stationIDs);
    }
    return true;
}

// PluginBase methods


void QuickBar::restoreState (const KConfigGroup &config)
{
    WidgetPluginBase::restoreState(config, false);

    int nStations = config.readEntry("nStations", 0);
    m_stationIDs.clear();
    for (int i = 1; i <= nStations; ++i) {
        QString s = config.readEntry(QString("stationID-") + QString().setNum(i), QString());
        if (s.length())
            m_stationIDs += s;
    }

    rebuildGUI();
    notifyStationSelectionChanged(m_stationIDs);
}


void QuickBar::saveState (KConfigGroup &config) const
{
    WidgetPluginBase::saveState(config);

    config.writeEntry("nStations", m_stationIDs.size());
    int i = 1;
    QStringList::const_iterator end = m_stationIDs.end();
    for (QStringList::const_iterator it = m_stationIDs.begin(); it != end; ++it, ++i) {
        config.writeEntry(QString("stationID-") + QString().setNum(i), *it);
    }
}


ConfigPageInfo QuickBar::createConfigurationPage()
{
    QuickbarConfiguration *conf = new QuickbarConfiguration(NULL);
    connectI (conf);
    return ConfigPageInfo(
        conf,
        i18n("Quickbar"),
        i18n("Quickbar Configuration"),
        "view-list-icons"
    );
}


/*AboutPageInfo QuickBar::createAboutPage()
{*/
/*    KAboutData aboutData("kradio4",
                         NULL,
                         NULL,
                         I18N_NOOP("Quickback for KRadio"),
                         KAboutData::License_GPL,
                         "(c) 2002-2005 Martin Witte, Klas Kalass",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  "", "emw-kradio@nocabal.de");
    aboutData.addAuthor("Klas Kalass",   "", "klas.kalass@gmx.de");

    return AboutPageInfo(
              new KRadioAboutWidget(aboutData, KRadioAboutWidget::AbtTabbed),
              i18n("Quickbar"),
              i18n("Quickbar Plugin"),
              "view-list-icons"
           );*/
//     return AboutPageInfo();
// }


// IRadio methods

bool QuickBar::noticePowerChanged(bool /*on*/)
{
    activateCurrentButton();
    autoSetCaption();
    return true;
}


bool QuickBar::noticeStationChanged (const RadioStation &rs, int /*idx*/)
{
    if (!m_ignoreNoticeActivation)
        activateButton(rs);
    autoSetCaption();
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
void QuickBar::uncheckAllOtherButtons(const QToolButton *b)
{
    bool blocked = m_mapper.signalsBlocked();
    m_mapper.blockSignals(true);
    QToolButton *otherbtn;
    foreach (otherbtn, m_buttons) {
        if (otherbtn != b) {
            otherbtn->setChecked(false);
        }
    }
    m_mapper.blockSignals(blocked);
}

void QuickBar::buttonToggled(const QString &stationID)
{
    QToolButton *b = static_cast<QToolButton*>(m_mapper.mapping(stationID));

    if (b) {
        if (b->isChecked()) {
            uncheckAllOtherButtons(b);
            const StationList &sl  = queryStations();
            const RadioStation &rs = sl.stationWithID(stationID);
            bool old = m_ignoreNoticeActivation;
            m_ignoreNoticeActivation = true;
            sendActivateStation(rs);
            m_ignoreNoticeActivation = old;
            sendPowerOn();
        } else {
            if (queryIsPowerOn() && stationID == queryCurrentStation().stationID()) {
                sendPowerOff();
            }
        }
    } else {
        // something strange happened
    }
}


void QuickBar::activateCurrentButton()
{
    activateButton(queryCurrentStation());
}


void QuickBar::activateButton(const RadioStation &rs)
{
    QToolButton *b = NULL;
    if (rs.isValid() && queryIsPowerOn()) {
        b = static_cast<QToolButton*>(m_mapper.mapping(rs.stationID()));
        if (b) {
            b->setChecked(true);
        }
    }
    uncheckAllOtherButtons(b);
    autoSetCaption();
}


void QuickBar::rebuildGUI()
{
    if (m_layout) delete m_layout;

    QToolButton *btn;
    foreach (btn, m_buttons) {
        delete btn;
    }
    m_buttons.clear();

    m_layout = new ButtonFlowLayout4(this);
    m_layout->setMargin(1);
    m_layout->setSpacing(0);


    const StationList &stations = queryStations();

    QString stationID;
    foreach (stationID, m_stationIDs) {

        const RadioStation &rs = stations.stationWithID(stationID);
        if (! rs.isValid()) continue;

        QToolButton *b = new QToolButton(this);
        m_buttons.append(b);
        b->setCheckable(true);
        if (rs.iconName().length()) {
            b->setIcon(KIcon(rs.iconName()));
        } else {
            b->setText(m_showShortName ? rs.shortName() : rs.name());
        }

        b->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
        b->setToolTip(rs.longName());
        m_layout->addWidget(b);
        b->show();

        m_mapper.setMapping(b, stationID);
        QObject::connect(b, SIGNAL(toggled(bool)), &m_mapper, SLOT(map()));
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



void QuickBar::setVisible(bool v)
{
    pSetVisible(v);
    QWidget::setVisible(v);
}



void    QuickBar::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    WidgetPluginBase::pShowEvent(e);
}

void    QuickBar::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);
    WidgetPluginBase::pHideEvent(e);
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


void QuickBar::autoSetCaption()
{
    const RadioStation &rs = queryCurrentStation();
    const QString caption = queryIsPowerOn() && rs.isValid() ? i18n("KRadio4 Quickbar, %1", rs.longName()) : i18n("KRadio4 Quickbar");
    setWindowTitle(caption);
}

void QuickBar::dragEnterEvent(QDragEnterEvent* event)
{
    bool a = StationDragObject::canDecode(event->mimeData());
    if (a)
        IErrorLogClient::staticLogDebug(i18n("contentsDragEnterEvent accepted"));
    else
        IErrorLogClient::staticLogDebug(i18n("contentsDragEnterEvent rejected"));
    event->accept();
}

void QuickBar::dropEvent(QDropEvent* event)
{
    QStringList list;

    if (StationDragObject::decode(event->mimeData(), list)) {
        QStringList l = getStationSelection();
        for (QList<QString>::const_iterator it = list.begin(); it != list.end(); ++it)
            if (!l.contains(*it))
                l.append(*it);
        setStationSelection(l);
    }
}


#include "quickbar.moc"
