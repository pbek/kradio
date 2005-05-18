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
#include <qtooltip.h>
#include <qcheckbox.h>
#include <qimage.h>

#include <kcombobox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kapplication.h>
#include <kwin.h>
#include <kconfig.h>
#include <kpopupmenu.h>

#include "../../src/interfaces/radiodevice_interfaces.h"
#include "../../src/radio-stations/radiostation.h"
#include "../../src/libkradio/stationlist.h"
#include "../../src/libkradio/pluginmanager.h"
#include "../../src/libkradio/plugin_configuration_dialog.h"
#include "../../src/libkradio-gui/aboutwidget.h"

#include "radioview.h"
#include "radioview_frequencyradio.h"
#include "radioview_volume.h"
#include "radioview_frequencyseeker.h"
#include "radioview-configuration.h"

#include <kaboutdata.h>

#define POPUP_ID_START_RECORDING_DEFAULT  0
#define POPUP_ID_STOP_RECORDING_BASE      100

///////////////////////////////////////////////////////////////////////

PLUGIN_LIBRARY_FUNCTIONS(RadioView, "Standard Display for KRadio");

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

RadioView::RadioView(const QString &name)
  : QWidget(NULL, name.ascii()),
    WidgetPluginBase(name, i18n("Radio Display")),
    enableToolbarFlag(false),
    btnPower(NULL),
    btnConfigure(NULL),
    btnQuit(NULL),
    btnRecording(NULL),
    btnSnooze(NULL),
    btnPlugins(NULL),
    comboStations(NULL),
    currentDevice(NULL),
    m_RecordingMenu(NULL),
    m_NextRecordingMenuID(POPUP_ID_STOP_RECORDING_BASE),
    m_PluginMenu(NULL)
{
    for (int i = 0; i < clsClassMAX; ++i)
        maxUsability[i] = 0;

    QBoxLayout *l01 = new QBoxLayout(this, QBoxLayout::LeftToRight, /*spacing=*/3);
    l01->setMargin(2);
    widgetStacks[clsRadioSound] = new QWidgetStack (this);
    l01->addWidget(widgetStacks[clsRadioSound]);

    QBoxLayout *l02 = new QBoxLayout(l01, QBoxLayout::Down);
    QBoxLayout *l03 = new QBoxLayout(l02, QBoxLayout::LeftToRight);
    comboStations = new KComboBox (this);
    l02->addWidget (comboStations);

    QBoxLayout *l05 = new QBoxLayout(l03, QBoxLayout::Down);
    widgetStacks[clsRadioDisplay] = new QWidgetStack (this);
    l05->addWidget(widgetStacks[clsRadioDisplay]);
    widgetStacks[clsRadioSeek] = new QWidgetStack (this);
    l05->addWidget(widgetStacks[clsRadioSeek]);

    QGridLayout *l04 = new QGridLayout (l03, /*rows=*/ 3, /*cols=*/ 2);
    btnPower         = new QToolButton(this);
    btnPower->setToggleButton(true);
    btnRecording     = new QToolButton(this);
    btnRecording->setToggleButton(true);
    btnConfigure     = new QToolButton(this);
    btnConfigure->setToggleButton(true);
    btnQuit          = new QToolButton(this);
    btnSnooze        = new QToolButton(this);
    btnSnooze->setToggleButton(true);
    btnPlugins       = new QToolButton(this);
    btnPlugins->setPopupDelay(1);
    l04->addWidget (btnPower,     0, 0);
    l04->addWidget (btnRecording, 0, 1);
    l04->addWidget (btnConfigure, 1, 0);
    l04->addWidget (btnQuit,      1, 1);
    l04->addWidget (btnSnooze,    2, 0);
    l04->addWidget (btnPlugins,   2, 1);

    m_pauseMenu     = new KPopupMenu(btnPower);
    m_pauseMenu->insertItem(SmallIcon("player_pause"),
                            i18n("Pause KRadio"),
                            this, SLOT(slotPause()));
    btnPower->setPopupDelay(250);

    m_RecordingMenu = new KPopupMenu(btnRecording);
    m_RecordingMenu->insertItem(SmallIcon("kradio_record"),
                                i18n("Start Recording"),
                                POPUP_ID_START_RECORDING_DEFAULT);
    QObject::connect(m_RecordingMenu, SIGNAL(activated(int)),
                     this,            SLOT(slotRecordingMenu(int)));
    btnRecording->setPopup(m_RecordingMenu);

    // Plugin-Button/Menu

    m_PluginMenu = new KPopupMenu(btnPlugins);
    if (m_manager)
        m_manager->addWidgetPluginMenuItems(m_PluginMenu, m_Plugins2MenuID);
    btnPlugins->setPopup(m_PluginMenu);

    // ICONS

    btnPower->setIconSet(SmallIconSet("kradio_muteon"));
    btnRecording->setIconSet(SmallIconSet("kradio_record"));
    btnConfigure->setIconSet(SmallIconSet("configure"));
    btnQuit->setIconSet(SmallIconSet("exit"));
    btnSnooze->setIconSet(SmallIconSet("kradio-zzz"));
    btnPlugins->setIconSet(SmallIconSet("kdf"));

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
    QObject::connect(btnRecording,  SIGNAL(clicked()),
                     this,          SLOT(slotRecord()));
    QObject::connect(btnSnooze,     SIGNAL(toggled(bool)),
                     this,          SLOT(slotSnooze(bool)));
    QObject::connect(comboStations, SIGNAL(activated(int)),
                     this,          SLOT(slotComboStationSelected(int)));
    QObject::connect(btnPlugins,    SIGNAL(clicked()),
                     this,          SLOT(slotBtnPluginsClicked()));

    // tooltips

    QToolTip::add(btnConfigure,  i18n("Configure KRadio"));
    QToolTip::add(btnPower,      i18n("Power On/Off"));
    QToolTip::add(btnQuit,       i18n("Quit KRadio Application"));
    QToolTip::add(btnRecording,  i18n("Start/Stop Recording"));
    QToolTip::add(btnSnooze,     i18n("Start/Stop Sleep Countdown"));
    QToolTip::add(btnPlugins,    i18n("Show/Hide Plugins"));
    QToolTip::add(comboStations, i18n("Select a Radio Station"));

    // testing
    addElement (new RadioViewFrequencyRadio (this, QString::null));
    addElement (new RadioViewVolume(this, QString::null));
    addElement (new RadioViewFrequencySeeker(this, QString::null));

    autoSetCaption();
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
    if (currentDevice)
        e->connectI(currentDevice);

    e->connectI(getSoundStreamServer());

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

    e->disconnectI(getSoundStreamServer());

    if (currentDevice)
        e->disconnectI(currentDevice);

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
    btnPower->setIconSet(SmallIconSet( on ? "kradio_muteoff" : "kradio_muteon"));
    btnPower->setOn(on);
    if (on) {
        btnPower->setPopup(m_pauseMenu);
    } else {
        btnPower->setPopup(NULL);
    }
    autoSetCaption();
    return true;
}


bool RadioView::noticeStationChanged (const RadioStation &, int idx)
{
    // add 1 for "no preset defined" entry
    comboStations->setCurrentItem(idx + 1);
    autoSetCaption();
    bool r = false;
    queryIsRecordingRunning(queryCurrentSoundStreamID(), r);
    m_RecordingMenu->setItemEnabled(POPUP_ID_START_RECORDING_DEFAULT, !r);
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
            QImage img(icon);
            int   h = img.height();
            float f = (float)(comboStations->height() - 4) / (h ? (float)h : 1.0);
            comboStations->insertItem(img.smoothScale((int)(img.width()*f), (int)(h * f)), stn->name());
        } else {
            comboStations->insertItem(stn->name());
        }
    }

    noticeStationChanged(queryCurrentStation(), queryCurrentStationIdx());
    return true;
}


bool RadioView::noticeCurrentSoundStreamIDChanged(SoundStreamID /*id*/)
{
    // FIXME: perhaps do something
    return false;
}

// IRadioDevicePoolClient

bool RadioView::noticeActiveDeviceChanged(IRadioDevice *newDevice)
{
    IRadioDevice *oldDevice = currentDevice;
    currentDevice = newDevice;

    for (ElementListIterator i(elements); i.current(); ++i) {
        RadioViewElement *e = i.current();
        if (oldDevice)
            e->disconnectI(oldDevice);
        if (newDevice)
            e->connectI(currentDevice);
    }

    selectTopWidgets();
    return true;
}


// Interface

bool RadioView::connectI(Interface *i)
{
    bool a = IRadioClient::connectI(i);
    bool b = IRadioDevicePoolClient::connectI(i);
    bool c = PluginBase::connectI(i);
    bool d = ITimeControlClient::connectI(i);

    // Callbacks for ISoundStreamClient

    bool e = ISoundStreamClient::connectI(i);
    if (e) {
        getSoundStreamServer()->register4_sendStartRecordingWithFormat(this);
        getSoundStreamServer()->register4_sendStopRecording           (this);
        getSoundStreamServer()->register4_notifySoundStreamChanged    (this);

        // special task for soundstreamclient, different from radio device pool
        for (ElementListIterator it(elements); it.current(); ++it) {
            RadioViewElement *e = it.current();
            e->connectI(i);
        }
    }

    return a || b || c || d || e;
}


bool RadioView::disconnectI(Interface *i)
{
    bool a = IRadioClient::disconnectI(i);
    bool b = IRadioDevicePoolClient::disconnectI(i);
    bool c = PluginBase::disconnectI(i);
    bool d = ITimeControlClient::disconnectI(i);
    bool e = ISoundStreamClient::disconnectI(i);
    if (e) {
        // special task for soundstreamclient, different from radio device pool
        for (ElementListIterator it(elements); it.current(); ++it) {
            RadioViewElement *e = it.current();
            e->disconnectI(i);
        }
    }
    return a || b || c || d || e;
}

// ISoundStreamClient

bool RadioView::startRecordingWithFormat(
    SoundStreamID      id,
    const SoundFormat &/*proposed_format*/,
    SoundFormat       &/*real_format*/)
{
    if (!id.isValid() || id != queryCurrentSoundStreamID() || m_StreamID2MenuID.contains(id))
        return false;

    QString descr;
    querySoundStreamDescription(id, descr);
    int menu_id = m_NextRecordingMenuID++;
    m_RecordingMenu->insertItem(SmallIcon("kradio_record"),
                                i18n("Stop Recording of %1").arg(descr),
                                menu_id);
    m_MenuID2StreamID.insert(menu_id, id);
    m_StreamID2MenuID.insert(id, menu_id);
    btnRecording->setOn(true);

    if (id == queryCurrentSoundStreamID())
        m_RecordingMenu->setItemEnabled(POPUP_ID_START_RECORDING_DEFAULT, false);

    return false; // this is only a "hook" that does not initiate the recording so don't say that we handled the event
}


bool RadioView::stopRecording (SoundStreamID id)
{
    if (!id.isValid() || !m_StreamID2MenuID.contains(id))
        return false;

    int menu_id = m_StreamID2MenuID[id];
    m_RecordingMenu->removeItem(menu_id);
    m_MenuID2StreamID.remove(menu_id);
    m_StreamID2MenuID.remove(id);
    btnRecording->setOn(m_StreamID2MenuID.count() > 0);

    if (id == queryCurrentSoundStreamID())
        m_RecordingMenu->setItemEnabled(POPUP_ID_START_RECORDING_DEFAULT, true);

    return false;
}


bool RadioView::noticeSoundStreamChanged(SoundStreamID id)
{
    if (m_StreamID2MenuID.contains(id)) {
        QString descr;
        querySoundStreamDescription(id, descr);
        m_RecordingMenu->changeItem(m_StreamID2MenuID[id],
                                    SmallIcon("kradio_record"),
                                    i18n("Stop Recording of %1").arg(descr));
        return true;
    }
    return false;
}


// ITimeControl

bool RadioView::noticeCountdownStarted(const QDateTime &)
{
    btnSnooze->setOn(true);
    return true;
}

bool RadioView::noticeCountdownStopped()
{
    btnSnooze->setOn(false);
    return true;
}

bool RadioView::noticeCountdownZero()
{
    btnSnooze->setOn(false);
    return true;
}

// WidgetPluginBase

void   RadioView::saveState (KConfig *config) const
{
    config->setGroup(QString("radioview-") + name());

    config->writeEntry("enableToobarFlag", enableToolbarFlag);
    WidgetPluginBase::saveState(config);

    for (ElementListIterator i(elements); i.current(); ++i) {
        RadioViewElement *e  = i.current();
        e->saveState(config);
    }
}


void   RadioView::restoreState (KConfig *config)
{
    config->setGroup(QString("radioview-") + name());

    enableToolbarFlag = config->readBoolEntry("enableToolbarFlag", false);
    WidgetPluginBase::restoreState(config);

    for (ElementListIterator i(elements); i.current(); ++i) {
        RadioViewElement *e  = i.current();
        e->restoreState(config);
    }
}


ConfigPageInfo RadioView::createConfigurationPage()
{
    RadioViewConfiguration *c = new RadioViewConfiguration();

    //addCommonConfigurationTab(c);

    for (ElementListIterator i(elements); i.current(); ++i) {
        addConfigurationTabFor(i.current(), c);
    }

    configPages.append(c);
    QObject::connect(c,    SIGNAL(destroyed(QObject *)),
                     this, SLOT(slotConfigPageDeleted(QObject *)));

    return ConfigPageInfo(
        c,
        i18n("Display"),
        i18n("Display Configuration"),
        "openterm"
    );
}


void RadioView::addConfigurationTabFor(RadioViewElement *e, QTabWidget *c)
{
    if (!e || !c)
        return;

    ConfigPageInfo inf = e->createConfigurationPage();

    if (inf.page) {

        if (inf.iconName.length()) {
            c->addTab(inf.page, QIconSet(SmallIconSet(inf.iconName)), inf.itemName);
        } else {
            c->addTab(inf.page, inf.itemName);
        }

        elementConfigPages.push_back(ElementCfg(e, inf.page));
        QObject::connect(inf.page, SIGNAL(destroyed(QObject *)),
                         this, SLOT(slotElementConfigPageDeleted(QObject *)));
    }
}


void RadioView::addCommonConfigurationTab(QTabWidget *c)
{
    if (!c)
        return;

    QFrame      *f = new QFrame(c);
    QVBoxLayout *l = new QVBoxLayout(f, 10);

    l->addWidget(new QCheckBox(i18n("set Toolbar-Flag for Display"), f));
    l->addItem(new QSpacerItem(1, 3, QSizePolicy::Fixed, QSizePolicy::Expanding));

    c->addTab(f, i18n("Common"));

    elementConfigPages.push_back(ElementCfg(f));
    QObject::connect(f,    SIGNAL(destroyed(QObject *)),
                     this, SLOT(slotElementConfigPageDeleted(QObject *)));
}


AboutPageInfo RadioView::createAboutPage()
{
    KAboutData aboutData("kradio",
                         NULL,
                         NULL,
                         I18N_NOOP("Standard Radio Display for KRadio"),
                         KAboutData::License_GPL,
                         "(c) 2002, 2003 Martin Witte, Klas Kalass",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  "", "witte@kawo1.rwth-aachen.de");
    aboutData.addAuthor("Klas Kalass",   "", "klas.kalass@gmx.de");

    return AboutPageInfo(
              new KRadioAboutWidget(aboutData, KRadioAboutWidget::AbtTabbed),
              i18n("Display"),
              i18n("Standard Radio Display for KRadio"),
              "openterm"
           );
}


void RadioView::noticeWidgetPluginShown(WidgetPluginBase *p, bool shown)
{
    if (!m_manager || !p)
        return;
    if ((WidgetPluginBase*)m_manager->getConfigDialog() == p) {
        btnConfigure->blockSignals(true);
        btnConfigure->setOn(shown);
        btnConfigure->blockSignals(false);
    }

    if (m_Plugins2MenuID.contains(p)) {
        m_manager->updateWidgetPluginMenuItem(p, m_PluginMenu, m_Plugins2MenuID, shown);
    }
}


// own Stuff

void RadioView::noticePluginsChanged(const PluginList &/*l*/)
{
    m_Plugins2MenuID.clear();
    m_PluginMenu->clear();
    if (m_manager)
        m_manager->addWidgetPluginMenuItems(m_PluginMenu, m_Plugins2MenuID);
}


void RadioView::slotPower(bool on)
{
    on ? sendPowerOn() : sendPowerOff();
    btnPower->setOn(queryIsPowerOn());
}


void RadioView::slotPause()
{
    if (queryIsPowerOn()) {
        sendPausePlayback(queryCurrentSoundStreamID());
    }
}


void RadioView::slotConfigure(bool b)
{
    QWidget *w = m_manager ? m_manager->getConfigDialog() : NULL;
    if (w) b ? w->show() : w->hide();
    if (!w)
        btnConfigure->setOn(false);
}


void RadioView::slotRecord()
{
    SoundStreamID id = queryCurrentSoundStreamID();
    bool b = btnRecording->isOn();

    bool r = false;
    queryIsRecordingRunning(id, r);

    if (!r && b /*!m_StreamID2MenuID.contains(id)*/) {
        sendStartRecording(id);
    } else if (r && !b) {
        sendStopRecording(id);
    }
}


void RadioView::slotRecordingMenu(int i)
{
    if (i == POPUP_ID_START_RECORDING_DEFAULT) {
        SoundStreamID id = queryCurrentSoundStreamID();
        bool          r  = false;
        queryIsRecordingRunning(id, r);
        if (!r)
            sendStartRecording(id);
    } else if (m_MenuID2StreamID.contains(i)) {
        sendStopRecording(m_MenuID2StreamID[i]);
    }
}


void RadioView::slotSnooze(bool on)
{
    if (on)
        sendStartCountdown();
    else
        sendStopCountdown();
}

void RadioView::slotComboStationSelected(int idx)
{
    if (idx > 0) {
        sendActivateStation(idx - 1);
    } else {
        comboStations->setCurrentItem(queryCurrentStationIdx() + 1);
    }
}

void RadioView::slotBtnPluginsClicked()
{
    btnPlugins->openPopup();
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


void RadioView::show()
{
    if (enableToolbarFlag)
        KWin::setType(winId(), NET::Toolbar);
    else
        KWin::setType(winId(), NET::Normal);
    WidgetPluginBase::pShow();
    QWidget::show();
}


void RadioView::hide()
{
    WidgetPluginBase::pHide();
    QWidget::hide();
}


void RadioView::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    WidgetPluginBase::pShowEvent(e);
}


void RadioView::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);
    WidgetPluginBase::pHideEvent(e);
}


void RadioView::autoSetCaption()
{
    const RadioStation &rs = queryCurrentStation();
    setCaption((queryIsPowerOn() && rs.isValid()) ? rs.longName() : QString("KRadio"));
}




#include "radioview.moc"
