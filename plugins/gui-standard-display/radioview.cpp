/***************************************************************************
                          radioview.cpp  -  description
                             -------------------
    begin                : Mit Mai 28 2003
    copyright            : (C) 2003 by Martin Witte
    email                : emw-kradio@nocabal.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QLayout>
#include <QToolButton>
#include <QSlider>
#include <QFile>
#include <QCheckBox>
#include <QImage>
#include <QStackedWidget>
#include <QGridLayout>
#include <QApplication>
#include <QMenu>

#include <kcmdlineargs.h>
#include <kcombobox.h>
#include <kicon.h>
#include <klocalizedstring.h>
#include <kwindowsystem.h>
#include <kconfiggroup.h>
#include <kmenu.h>

#include "radiodevice_interfaces.h"
#include "radiostation.h"
#include "stationlist.h"
#include "pluginmanager.h"
#include "plugin_configuration_dialog.h"

#include "radioview.h"
#include "radioview_frequencyradio.h"
#include "radioview_volume.h"
#include "radioview_frequencyseeker.h"
#include "radioview-configuration.h"

//#include <kaboutdata.h>

///////////////////////////////////////////////////////////////////////

static KAboutData aboutData()
{
    KAboutData about("RadioView",
                     PROJECT_NAME,
                     ki18nc("@title", "Standard Display"),
                     KRADIO_VERSION,
                     ki18nc("@title", "Standard Radio Display"),
                     KAboutData::License_GPL,
                     ki18nc("@info:credit", "(c) 2002-2005 Martin Witte, Klas Kalass"),
                     KLocalizedString(),
                     "http://sourceforge.net/projects/kradio",
                     "emw-kradio@nocabal.de");
    about.addAuthor(ki18nc("@info:credit", "Martin Witte"), KLocalizedString(), "emw-kradio@nocabal.de");
    about.addAuthor(ki18nc("@info:credit", "Klas Kalass"), KLocalizedString(), "klas.kalass@gmx.de");
    return about;
}

KRADIO_EXPORT_PLUGIN(RadioView, aboutData())

///////////////////////////////////////////////////////////////////////

RadioView::RadioView(const QString &instanceID, const QString &name)
  : QWidget(NULL),
    WidgetPluginBase(this, instanceID, name, i18n("Radio Display")),
    enableToolbarFlag(false),
    btnPower(NULL),
    btnConfigure(NULL),
    btnQuit(NULL),
    btnRecording(NULL),
    btnSnooze(NULL),
    btnPlugins(NULL),
    btnHelp(NULL),
    comboStations(NULL),
    m_ConfigPage(NULL),
    currentDevice(NULL),
    m_RecordingMenu(NULL),
    m_recordingDefaultMenuItem(NULL),
    m_helpMenu(NULL, KCmdLineArgs::aboutData())
{
    for (int i = 0; i < clsClassMAX; ++i)
        maxUsability[i] = 0;

    QHBoxLayout *l01 = new QHBoxLayout(this);
    l01->setMargin(1);
    l01->setSpacing(0);
    widgetStacks[clsRadioSound] = new QStackedWidget (this);
    l01->addWidget(widgetStacks[clsRadioSound]);

    QVBoxLayout *l02 = new QVBoxLayout();
    l02->setSpacing(0);
    l01->addLayout(l02);

    QHBoxLayout *l03 = new QHBoxLayout();
    l03->setSpacing(0);
    l02->addLayout(l03);

    QHBoxLayout *l06 = new QHBoxLayout();
    l06->setSpacing(0);
    l02->addLayout(l06);

    comboStations = new KComboBox (this);
    l06->addWidget (comboStations);

    QVBoxLayout *l05 = new QVBoxLayout();
    l03->addLayout(l05);

    widgetStacks[clsRadioDisplay] = new QStackedWidget (this);
    l05->addWidget(widgetStacks[clsRadioDisplay]);
    widgetStacks[clsRadioSeek] = new QStackedWidget(this);
    l05->addWidget(widgetStacks[clsRadioSeek]);

    QGridLayout *l04 = new QGridLayout ();
    l04->setMargin(0);
    l04->setSpacing(0);
    l03->addLayout(l04);

    btnPower         = new QToolButton(this);
    btnRecording     = new QToolButton(this);
    btnConfigure     = new QToolButton(this);
    btnQuit          = new QToolButton(this);
    btnSnooze        = new QToolButton(this);
    btnPlugins       = new QToolButton(this);
    btnHelp          = new QToolButton(this);

    btnPower    ->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    btnRecording->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    btnConfigure->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    btnQuit     ->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    btnSnooze   ->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    btnPlugins  ->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    btnHelp     ->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    btnPower    ->setCheckable(true);
    btnRecording->setCheckable(true);
    btnConfigure->setCheckable(true);
    btnSnooze   ->setCheckable(true);

//     btnPlugins  ->setPopupDelay(1);

    l04->addWidget (btnPower,     0, 0);
    l04->addWidget (btnRecording, 0, 1);
    l04->addWidget (btnConfigure, 1, 0);
    l04->addWidget (btnQuit,      1, 1);
    l04->addWidget (btnSnooze,    2, 0);
    l04->addWidget (btnPlugins,   2, 1);
    l06->addWidget (btnHelp);

    m_pauseMenu     = new QMenu(this);
    m_pauseMenuItem = m_pauseMenu->addAction(KIcon("media-playback-pause"), i18n("Pause KRadio"));
    QObject::connect(m_pauseMenuItem, SIGNAL(triggered()), this, SLOT(slotPause()));
    btnPower->setMenu(m_pauseMenu);

    m_RecordingMenu            = new QMenu(btnRecording);
    m_recordingDefaultMenuItem = m_RecordingMenu->addAction(KIcon("media-record"), i18n("Start Recording"));
    QObject::connect(m_RecordingMenu,            SIGNAL(triggered(QAction *)), this, SLOT(slotRecordingMenu(QAction *)));
    QObject::connect(m_recordingDefaultMenuItem, SIGNAL(triggered()),          this, SLOT(slotStartDefaultRecording()));
    btnRecording->setMenu(m_RecordingMenu);

    m_SnoozeMenu   = new QMenu(btnSnooze);
    QObject::connect(m_SnoozeMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotSnooze(QAction*)));

    QAction *a = NULL;
    a = m_SnoozeMenu->addAction(i18n("5 min"));
    a->setData(5);

    a = m_SnoozeMenu->addAction(i18n("10 min"));
    a->setData(10);

    a = m_SnoozeMenu->addAction(i18n("15 min"));
    a->setData(15);

    a = m_SnoozeMenu->addAction(i18n("30 min"));
    a->setData(30);

    a = m_SnoozeMenu->addAction(i18n("60 min"));
    a->setData(60);

    a = m_SnoozeMenu->addAction(i18n("90 min"));
    a->setData(90);

    a = m_SnoozeMenu->addAction(i18n("120 min"));
    a->setData(120);

    btnSnooze->setMenu(m_SnoozeMenu);
//     btnSnooze->setPopupDelay(200);


    // ICONS

    btnPower    ->setIcon(KIcon("media-playback-start"));
    btnRecording->setIcon(KIcon("media-record"));
    btnConfigure->setIcon(KIcon("configure"));
    btnQuit     ->setIcon(KIcon("application-exit"));
    btnSnooze   ->setIcon(KIcon("kradio_zzz"));
    btnPlugins  ->setIcon(KIcon("preferences-plugin"));
    btnHelp     ->setIcon(KIcon("help-about"));
    btnHelp     ->setMenu(m_helpMenu.menu());

    widgetStacks[clsRadioSound]  ->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,              QSizePolicy::Preferred));
    //widgetStacks[clsRadioDisplay]->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
    widgetStacks[clsRadioSeek]   ->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,   QSizePolicy::Fixed));
    comboStations                ->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,          QSizePolicy::Fixed));
    comboStations->setMinimumHeight(28);


    QObject::connect(btnPower,      SIGNAL(toggled(bool)),
                     this,          SLOT(slotPower(bool)));
    QObject::connect(btnQuit,       SIGNAL(clicked()),
                     qApp,          SLOT(quit()));
    QObject::connect(btnConfigure,  SIGNAL(toggled(bool)),
                     this,          SLOT(slotConfigure(bool)));
    QObject::connect(btnRecording,  SIGNAL(clicked()),
                     this,          SLOT(slotRecord()));
    QObject::connect(btnSnooze,     SIGNAL(toggled(bool)),
                     this,          SLOT(slotSnooze(bool)));
    QObject::connect(comboStations, SIGNAL(activated(int)),
                     this,          SLOT(slotComboStationSelected(int)));
    QObject::connect(btnPlugins,    SIGNAL(clicked()),
                     btnPlugins,    SLOT(showMenu()));
    QObject::connect(btnHelp,       SIGNAL(clicked()),
                     btnHelp,       SLOT(showMenu()));

    // tooltips

    btnConfigure ->setToolTip(i18n("Configure KRadio"));
    btnPower     ->setToolTip(i18n("Power on/off"));
    btnQuit      ->setToolTip(i18n("Quit KRadio application"));
    btnRecording ->setToolTip(i18n("Start/stop recording"));
    btnSnooze    ->setToolTip(i18n("Start/stop sleep countdown"));
    btnPlugins   ->setToolTip(i18n("Show/hide plugins"));
    btnHelp      ->setToolTip(i18n("Help about KRadio and KDE"));
    comboStations->setToolTip(i18n("Select a radio station"));

    // testing
    addElement (new RadioViewFrequencyRadio (this, QString()));
    addElement (new RadioViewVolume         (this, QString()));
    addElement (new RadioViewFrequencySeeker(this, QString()));

    autoSetCaption();

    m_WorkaroundRecordingMenuUpdate.setInterval(100);
    m_WorkaroundRecordingMenuUpdate.setSingleShot(true);
    QObject::connect(&m_WorkaroundRecordingMenuUpdate, SIGNAL(timeout()), this, SLOT(slotUpdateRecordingMenu()));
}


RadioView::~RadioView ()
{
    qDeleteAll(m_elementConfigPages);
    m_elementConfigPages.clear();

    if (m_RecordingMenu) delete m_RecordingMenu;
    if (m_pauseMenu)     delete m_pauseMenu;
    if (m_SnoozeMenu)    delete m_SnoozeMenu;
    m_RecordingMenu = NULL;
    m_pauseMenu     = NULL;
    m_SnoozeMenu    = NULL;
}


bool RadioView::addElement (RadioViewElement *e)
{
    if (!e) return false;

    RadioViewClass cls = e->getClass();

    if (cls < 0 || cls >= clsClassMAX)
        return false;


    e->setParent(this);
    e->move(0, 0);
    e->show();

    QObject::connect(e,    SIGNAL(destroyed(QObject*)),
                     this, SLOT(removeElement(QObject*)));
    m_elementConfigPages.insert(e, NULL);
    widgetStacks[cls]->addWidget(e);

    // connect Element with device, disconnect doesn't matter (comp. removeElement)
    // other devices follow if currentDevice changes
    if (currentDevice)
        e->connectI(currentDevice);

    e->connectI(getSoundStreamServer());

    addConfigurationTabFor(e, m_ConfigPage);

    selectTopWidgets();

    return true;
}


bool RadioView::removeElement (QObject *_e)
{
    if (!_e)
        return false;

    QObject::disconnect(_e,   SIGNAL(destroyed(QObject*)),
                        this, SLOT(removeElement(QObject*)));

    if (m_elementConfigPages.contains(_e)) {
        delete m_elementConfigPages[_e];
        m_elementConfigPages[_e] = NULL;
    }
//     ElementCfgListIterator it;
//
//     while ((it = elementConfigPages.find(e)) != elementConfigPages.end()) {
//         delete (*it).cfg;
//         // it must not used behind, the element will be deleted automatically
//         // by slotElementConfigPageDeleted
//     }

    RadioViewElement *e = dynamic_cast<RadioViewElement*>(_e);

    if (e) {
        e->disconnectI(getSoundStreamServer());

        if (currentDevice)
            e->disconnectI(currentDevice);

        RadioViewClass cls = e->getClass();
        widgetStacks[cls]->removeWidget(e);
    //     elements.remove(e);
    }

    m_elementConfigPages.remove(_e);
    selectTopWidgets();

    return true;
}


void RadioView::selectTopWidgets()
{
    for (int i = 0; i < clsClassMAX; ++i)
        maxUsability[i] = 0;

    QObject *o = NULL;
    foreach(o, m_elementConfigPages.keys()) {
        RadioViewElement *e = dynamic_cast<RadioViewElement *>(o);
        if (e) {
            RadioViewClass   cls = e->getClass();
            float u = e->getUsability(currentDevice);
            if (u > maxUsability[cls]) {
                maxUsability[cls] = u;
                e->setEnabled(true);
                widgetStacks[cls]->setCurrentWidget(e);
            }
            else if (u <= 0) {
                e->setEnabled(false);
            }
        }
    }
    for (int cls = 0; cls < clsClassMAX; ++cls) {
        if (maxUsability[cls] <= 0) {
            widgetStacks[cls]->setEnabled(false);
        } else {
            widgetStacks[cls]->setEnabled(true);
        }
    }
    // adjustLayout!?
}


// IRadioClient

bool RadioView::noticePowerChanged(bool on)
{
    btnPower->setIcon(KIcon( on ? "media-playback-stop" : "media-playback-start"));
    btnPower->setChecked(on);
    if (on) {
        btnPower->setMenu(m_pauseMenu);
    } else {
        btnPower->setMenu(NULL);
    }
    updatePauseMenuItem(/*run query*/true, /*ignored*/false);
    autoSetCaption();
    return true;
}


bool RadioView::noticeStationChanged (const RadioStation &, int idx)
{
    // add 1 for "no preset defined" entry
    comboStations->setCurrentIndex(idx + 1);
    autoSetCaption();
    bool r = false;
    SoundFormat   sf;
    queryIsRecordingRunning(queryCurrentSoundStreamSinkID(), r, sf);

    m_recordingDefaultMenuItem->setEnabled(!r);

    return true;
}


bool RadioView::noticeStationsChanged(const StationList &list)
{
    comboStations->clear();
    comboStations->addItem(i18n("<no preset defined>"));

    for (StationList::const_iterator it = list.begin(); it != list.end(); ++it) {
        RadioStation *stn = *it;
        QString icon = stn->iconName();
        if (icon.length() && QFile(icon).exists()) {
            QImage img(icon);
            int   h = img.height();
            float f = (float)(comboStations->height() - 4) / (h ? (float)h : 1.0);
            comboStations->addItem(QPixmap::fromImage(img.scaled((int)(img.width()*f), (int)(h * f))), stn->name());
        } else {
            comboStations->addItem(stn->name());
        }
    }

    noticeStationChanged(queryCurrentStation(), queryCurrentStationIdx());
    return true;
}




bool RadioView::noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/)
{
    // FIXME: perhaps do something
    return false;
}

bool RadioView::noticeCurrentSoundStreamSinkIDChanged(SoundStreamID /*id*/)
{
    // FIXME: perhaps do something
    return false;
}

// IRadioDevicePoolClient

bool RadioView::noticeActiveDeviceChanged(IRadioDevice *newDevice)
{
    IRadioDevice *oldDevice = currentDevice;
    currentDevice = newDevice;

    QObject *o = NULL;
    foreach(o, m_elementConfigPages.keys()) {
        RadioViewElement *e = dynamic_cast<RadioViewElement *>(o);
        if (e) {
            if (oldDevice)
                e->disconnectI(oldDevice);
            if (newDevice)
                e->connectI(currentDevice);
        }
    }

    selectTopWidgets();

// EMW: works in principle, but is not very nice from the visual point of view of the seekers are missing
//     if (dynamic_cast<ISeekRadio*>(newDevice)) {
//         widgetStacks[clsRadioSeek]->show();
//     } else {
//         widgetStacks[clsRadioSeek]->hide();
//     }
// 
    return true;
}


bool RadioView::setManager (PluginManager *m)
{
    bool r = PluginBase::setManager(m);
    if (m_manager && btnPlugins) {
        btnPlugins->setMenu(m_manager->getPluginHideShowMenu());
    }
    return r;
}

void RadioView::unsetManager ()
{
    PluginBase::unsetManager();
    if (btnPlugins) {
        btnPlugins->setMenu(NULL);
    }
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
        QObject *o = NULL;
        foreach(o, m_elementConfigPages.keys()) {
            RadioViewElement *el = dynamic_cast<RadioViewElement *>(o);
            if (el) {
                el->disconnectI(i);
            }
        }
    }
    return a || b || c || d || e;
}

void RadioView::noticeConnectedI (ISoundStreamServer *s, bool pointer_valid)
{
    ISoundStreamClient::noticeConnectedI(s, pointer_valid);
    if (s && pointer_valid) {
        s->register4_sendStartRecordingWithFormat(this);
        s->register4_sendStopRecording           (this);
        s->register4_sendPausePlayback           (this);
        s->register4_sendResumePlayback          (this);
        s->register4_notifySoundStreamChanged    (this);

        updatePauseMenuItem(/*run query*/true, /*ignored*/false);

        // special task for soundstreamclient, different from radio device pool
        QObject *o = NULL;
        foreach(o, m_elementConfigPages.keys()) {
            RadioViewElement *el = dynamic_cast<RadioViewElement *>(o);
            if (el) {
                el->connectI(s);
            }
        }
    }
}

// ISoundStreamClient

bool RadioView::startRecordingWithFormat(
    SoundStreamID      id,
    const SoundFormat &/*proposed_format*/,
    SoundFormat       &/*real_format*/,
    const recordingTemplate_t     &/*filenameTemplate*/)
{
    if (!id.isValid() || id != queryCurrentSoundStreamSinkID() || m_StreamID2MenuID.contains(id))
        return false;

    QString descr;
    querySoundStreamDescription(id, descr);
    QAction *a = m_RecordingMenu->addAction(KIcon("media-record"), i18n("Stop Recording of %1", descr));
    a->setData(QVariant::fromValue(id));
    m_StreamID2MenuID.insert(id, a);
    btnRecording->setChecked(true);

    m_recordingDefaultMenuItem->setEnabled(false);

    return false; // this is only a "hook" that does not initiate the recording so don't say that we handled the event
}


bool RadioView::stopRecording (SoundStreamID id)
{
    if (!id.isValid() || !m_StreamID2MenuID.contains(id))
        return false;

    if (m_StreamID2MenuID.contains(id)) {
        QAction *a = m_StreamID2MenuID[id];
        m_StreamID2MenuID.remove(id);
        m_WorkaroundRecordingMenuActionsToBeDeleted.append(a);
        m_WorkaroundRecordingMenuUpdate.start();
    }
    btnRecording->setChecked(m_StreamID2MenuID.size() > 0);

    if (id == queryCurrentSoundStreamSinkID())
        m_recordingDefaultMenuItem->setEnabled(true);

    return false;
}


bool RadioView::pausePlayback(SoundStreamID id)
{
    if (queryCurrentSoundStreamSinkID() == id) {
        updatePauseMenuItem(/*run query*/false, /*pause state*/true);
    }
    return false; // we always return false to indicate, that this is just a hook to get to know that something was paused
}

bool RadioView::resumePlayback(SoundStreamID id)
{
    if (queryCurrentSoundStreamSinkID() == id) {
        updatePauseMenuItem(/*run query*/false, /*pause state*/false);
    }
    return false; // we always return false to indicate, that this is just a hook to get to know that something was paused
}

void RadioView::updatePauseMenuItem(bool run_query, bool known_pause_state)
{
    if (run_query) {
        queryIsPlaybackPaused(queryCurrentSoundStreamSinkID(), known_pause_state);
    }
    if (known_pause_state) {
        m_pauseMenuItem->setText(i18n("Resume playback"));
        m_pauseMenuItem->setIcon(KIcon("media-playback-start"));
    }
    else {
        m_pauseMenuItem->setText(i18n("Pause playback"));
        m_pauseMenuItem->setIcon(KIcon("media-playback-pause"));
    }
}


bool RadioView::noticeSoundStreamChanged(SoundStreamID id)
{
    if (m_StreamID2MenuID.contains(id)) {
        QAction *a = m_StreamID2MenuID[id];
        QString descr;
        querySoundStreamDescription(id, descr);
        a->setIcon(KIcon("media-record"));
        a->setText(i18n("Stop Recording of %1", descr));
        return true;
    }
    return false;
}


// ITimeControl

bool RadioView::noticeCountdownStarted(const QDateTime &)
{
    btnSnooze->setChecked(true);
    return true;
}

bool RadioView::noticeCountdownStopped()
{
    btnSnooze->setChecked(false);
    return true;
}

bool RadioView::noticeCountdownZero()
{
    btnSnooze->setChecked(false);
    return true;
}

// WidgetPluginBase

void   RadioView::saveState (KConfigGroup &config) const
{
    config.writeEntry("enableToobarFlag", enableToolbarFlag);
    WidgetPluginBase::saveState(config);

    QObject *o = NULL;
    foreach(o, m_elementConfigPages.keys()) {
        RadioViewElement *el = dynamic_cast<RadioViewElement *>(o);
        if (el) {
            el->saveState(config);
        }
    }
}


void   RadioView::restoreState (const KConfigGroup &config)
{
    enableToolbarFlag = config.readEntry("enableToolbarFlag", false);
    WidgetPluginBase::restoreState(config, true);

    QObject *o = NULL;
    foreach(o, m_elementConfigPages.keys()) {
        RadioViewElement *el = dynamic_cast<RadioViewElement *>(o);
        if (el) {
            el->restoreState(config);
        }
    }
}


ConfigPageInfo RadioView::createConfigurationPage()
{
    if (!m_ConfigPage) {
        m_ConfigPage = new RadioViewConfiguration();
    }

    addCommonConfigurationTab(m_ConfigPage);

    QObject *o = NULL;
    foreach(o, m_elementConfigPages.keys()) {
        RadioViewElement *el = dynamic_cast<RadioViewElement *>(o);
        if (el) {
            addConfigurationTabFor(el, m_ConfigPage);
        }
    }

    QObject::connect(m_ConfigPage, SIGNAL(destroyed(QObject *)),
                     this,         SLOT(slotConfigPageDeleted(QObject *)));

    return ConfigPageInfo(
        m_ConfigPage,
        i18n("Display"),
        i18n("Display Configuration"),
        "preferences-desktop-display"
    );
}


void RadioView::addConfigurationTabFor(RadioViewElement *e, RadioViewConfiguration *c)
{
    if (!e || !c)
        return;

    ConfigPageInfo inf = e->createConfigurationPage();

    if (inf.page) {

        if (inf.iconName.length()) {
            c->addElementTab(inf.page, KIcon(inf.iconName), inf.itemName);
        } else {
            c->addElementTab(inf.page, inf.itemName);
        }

        m_elementConfigPages.insert(e, inf.page);
        QObject::connect(inf.page, SIGNAL(destroyed(QObject *)),
                         this,     SLOT(slotElementConfigPageDeleted(QObject *)));
    }
}


void RadioView::addCommonConfigurationTab(RadioViewConfiguration */*c*/)
{
/*    if (!c)
        return;

    QWidget     *f = new QWidget(c);
    QVBoxLayout *l = new QVBoxLayout(f);
    l->setSpacing(10);

    l->addWidget(new QCheckBox(i18n("set Toolbar-Flag for Display"), f));
    l->addItem  (new QSpacerItem(1, 3, QSizePolicy::Fixed, QSizePolicy::Expanding));

    c->addElementTab(f, i18n("Common"));*/
}


void RadioView::noticeWidgetPluginShown(WidgetPluginBase *p, bool shown)
{
    if (!m_manager || !p)
        return;
    if (static_cast<WidgetPluginBase*>(m_manager->getConfigDialog()) == p) {
        btnConfigure->blockSignals(true);
        btnConfigure->setChecked(shown);
        btnConfigure->blockSignals(false);
    }
}


// own Stuff

void RadioView::slotPower(bool on)
{
    on ? sendPowerOn() : sendPowerOff();
    btnPower->setChecked(queryIsPowerOn());
}


void RadioView::slotPause()
{
    if (queryIsPowerOn()) {
        bool          paused = false;
        SoundStreamID id     = queryCurrentSoundStreamSinkID();
        queryIsPlaybackPaused(id, paused);
        if (paused) {
            sendResumePlayback(id);
        } else {
            sendPausePlayback(id);
        }
    }
}


void RadioView::slotConfigure(bool b)
{
    QWidget *w = m_manager ? m_manager->getConfigDialog() : NULL;
    if (w) b ? w->show() : w->hide();
    if (!w)
        btnConfigure->setChecked(false);
}


void RadioView::slotRecord()
{
    SoundStreamID id = queryCurrentSoundStreamSinkID();
    bool b = btnRecording->isChecked();

    bool r = false;
    SoundFormat   sf;
    queryIsRecordingRunning(id, r, sf);

    if (!r && b /*!m_StreamID2MenuID.contains(id)*/) {
        if (!queryIsPowerOn())
            sendPowerOn();
        sendStartRecording(id);
    } else if (r && !b) {
        sendStopRecording(id);
    }
}


void RadioView::slotStartDefaultRecording()
{
    SoundStreamID id = queryCurrentSoundStreamSinkID();
    bool          r  = false;
    SoundFormat   sf;
    queryIsRecordingRunning(id, r, sf);
    if (!r) {
        if (!queryIsPowerOn())
            sendPowerOn();
        sendStartRecording(id);
    }
}

void RadioView::slotRecordingMenu(QAction *a)
{
    const QVariant &data = a->data();
    if (!data.isNull() && data.isValid() && data.canConvert<SoundStreamID>()) {
        SoundStreamID id = data.value<SoundStreamID>();
        sendStopRecording(id);
    }
}


void RadioView::slotSnooze(bool on)
{
    if (on)
        sendStartCountdown();
    else
        sendStopCountdown();
}


void RadioView::slotSnooze(QAction *a)
{
    const QVariant &data = a->data();
    if (!data.isNull() && data.isValid() && data.canConvert<int>()) {
        sendCountdownSeconds(60 * data.toInt(), querySuspendOnSleep());
        sendStartCountdown();
    }
}


void RadioView::slotComboStationSelected(int idx)
{
    if (idx > 0) {
        sendActivateStation(idx - 1);
    } else {
        comboStations->setCurrentIndex(queryCurrentStationIdx() + 1);
    }
}

void RadioView::slotConfigPageDeleted(QObject *)
{
    m_ConfigPage = NULL;
}


void RadioView::slotElementConfigPageDeleted(QObject *p)
{
    if (p) {
        QObject *o = NULL;
        foreach(o, m_elementConfigPages.keys(p)) {
            m_elementConfigPages[o] = NULL;
        }
    }
}


void RadioView::setVisible(bool v)
{
    if (enableToolbarFlag)
        KWindowSystem::setType(winId(), NET::Toolbar);
    else
        KWindowSystem::setType(winId(), NET::Normal);
    pSetVisible(v);
    QWidget::setVisible(v);
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
    setWindowTitle((queryIsPowerOn() && rs.isValid()) ? rs.longName() : QString::fromLatin1("KRadio"));
}



void RadioView::slotUpdateRecordingMenu()
{
    qDeleteAll(m_WorkaroundRecordingMenuActionsToBeDeleted);
    m_WorkaroundRecordingMenuActionsToBeDeleted.clear();
}



#include "radioview.moc"
