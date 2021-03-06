/***************************************************************************
                          shortcuts.cpp  -  description
                             -------------------
    begin                : Mon Feb 4 2002
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

#include "shortcuts.h"
#include "widgetpluginbase.h"

#include <QSocketNotifier>
#include <QTimer>
#include <QtWidgets/QApplication>
#include <QtGui/QIcon>
#include <QtWidgets/QAction>
#include <QDebug>

#include <KAboutData>
#include <KShortcutsEditor>
#include <KGlobalAccel>

#include "errorlog_interfaces.h"
#include "radiodevice_interfaces.h"
#include "seekradio_interfaces.h"
#include "stationlist.h"
#include "radiostation.h"

#include "shortcuts-configuration.h"




///////////////////////////////////////////////////////////////////////
//// plugin library functions

static KAboutData aboutData()
{
    KAboutData about("Shortcuts",
                     i18nc("@title", "Shortcuts"),
                     KRADIO_VERSION,
                     i18nc("@title", "Shortcuts Support"),
                     KAboutLicense::LicenseKey::GPL,
                     nullptr,
                     nullptr,
                     "http://sourceforge.net/projects/kradio",
                     "emw-kradio@nocabal.de");
    return about;
}

KRADIO_EXPORT_PLUGIN(Shortcuts, aboutData())
#include "shortcuts.moc"

/////////////////////////////////////////////////////////////////////////////


Shortcuts::Shortcuts(const QString &instanceID, const QString &name)
    : PluginBase(instanceID, name, i18n("Shortcuts Plugin")),
      m_stdCollection    (nullptr),
      m_stationCollection(nullptr),
      m_stdActions       (nullptr),
      m_stationActions   (nullptr)
{
    m_stdCollection     = new KActionCollection(this);
    m_stationCollection = new KActionCollection(this);
    m_stdActions        = new KActionCategory(i18n("Standard Actions"), m_stdCollection);
    m_stationActions    = new KActionCategory(i18n("Station Actions"),  m_stationCollection);
    
    QObject::connect(m_stdCollection,     &KActionCollection::actionTriggered, this, &Shortcuts::slotActionTriggered);
    QObject::connect(m_stationCollection, &KActionCollection::actionTriggered, this, &Shortcuts::slotStationTriggered);


    addAction(i18n("Digit 0"),                 SHORTCUT_DIGIT_0,          QKeySequence(Qt::Key_0));
    addAction(i18n("Digit 1"),                 SHORTCUT_DIGIT_1,          QKeySequence(Qt::Key_1));
    addAction(i18n("Digit 2"),                 SHORTCUT_DIGIT_2,          QKeySequence(Qt::Key_2));
    addAction(i18n("Digit 3"),                 SHORTCUT_DIGIT_3,          QKeySequence(Qt::Key_3));
    addAction(i18n("Digit 4"),                 SHORTCUT_DIGIT_4,          QKeySequence(Qt::Key_4));
    addAction(i18n("Digit 5"),                 SHORTCUT_DIGIT_5,          QKeySequence(Qt::Key_5));
    addAction(i18n("Digit 6"),                 SHORTCUT_DIGIT_6,          QKeySequence(Qt::Key_6));
    addAction(i18n("Digit 7"),                 SHORTCUT_DIGIT_7,          QKeySequence(Qt::Key_7));
    addAction(i18n("Digit 8"),                 SHORTCUT_DIGIT_8,          QKeySequence(Qt::Key_8));
    addAction(i18n("Digit 9"),                 SHORTCUT_DIGIT_9,          QKeySequence(Qt::Key_9));
    addAction(i18n("Power on"),                SHORTCUT_POWER_ON,         QKeySequence(Qt::Key_P));
    addAction(i18n("Power off"),               SHORTCUT_POWER_OFF,        QKeySequence(Qt::CTRL + Qt::Key_P));
    addAction(i18n("Pause"),                   SHORTCUT_PAUSE,            QKeySequence(Qt::Key_Space));
    addAction(i18n("Start recording"),         SHORTCUT_RECORD_START,     QKeySequence(Qt::Key_R));
    addAction(i18n("Stop recording"),          SHORTCUT_RECORD_STOP,      QKeySequence(Qt::CTRL + Qt::Key_R));
    addAction(i18n("Increase volume"),         SHORTCUT_VOLUME_INC,       QKeySequence(Qt::Key_Up));
    addAction(i18n("Decrease volume"),         SHORTCUT_VOLUME_DEC,       QKeySequence(Qt::Key_Down));
    addAction(i18n("Increase Frequency"),      SHORTCUT_FREQ_INC,         QKeySequence(Qt::SHIFT + Qt::Key_Right));
    addAction(i18n("Decrease Frequency"),      SHORTCUT_FREQ_DEC,         QKeySequence(Qt::SHIFT + Qt::Key_Left));
    addAction(i18n("Next station"),            SHORTCUT_STATION_NEXT,     QKeySequence(Qt::Key_Right));
    addAction(i18n("Previous station"),        SHORTCUT_STATION_PREV,     QKeySequence(Qt::Key_Left));
    addAction(i18n("Search next station"),     SHORTCUT_SEARCH_NEXT,      QKeySequence(Qt::CTRL + Qt::Key_Right));
    addAction(i18n("Search previous station"), SHORTCUT_SEARCH_PREV,      QKeySequence(Qt::CTRL + Qt::Key_Left));
    addAction(i18n("Sleep"),                   SHORTCUT_SLEEP,            QKeySequence(Qt::CTRL + Qt::Key_Z));
    addAction(i18n("Quit KRadio"),             SHORTCUT_APPLICATION_QUIT, QKeySequence::Quit);

    m_kbdTimer = new QTimer (this);
    QObject::connect (m_kbdTimer, &QTimer::timeout, this, &Shortcuts::slotKbdTimedOut);

    m_addIndex = 0;

} // CTOR


Shortcuts::~Shortcuts()
{
    while (m_ShortcutsEditors.size()) {
        delete m_ShortcutsEditors.first(); // will be automatically removed from list by destroyed signal
    }
    if (m_stationActions) {
        delete m_stationActions;
        m_stationActions = nullptr;
    }
    if (m_stdActions) {
        delete m_stdActions;
        m_stdActions = nullptr;
    }
    if (m_stdCollection) {
        delete m_stdCollection;
        m_stdCollection = nullptr;
    }
    if (m_stationCollection) {
        delete m_stationCollection;
        m_stationCollection = nullptr;
    }
} // DTOR


void Shortcuts::addAction(const QString &name, ShortcutID id, const QKeySequence & keyseq)
{
    QAction *a = m_stdActions->addAction(name);
    a->setData           ((int)id);
    a->setText           (name);
    a->setShortcut       (keyseq);
    m_stdActions->collection()->setDefaultShortcut(a, keyseq);
    a->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    a->setEnabled        (true);
    KGlobalAccel::setGlobalShortcut(a, QKeySequence());
} // addAction



void Shortcuts::slotKbdTimedOut()
{
    activateStation (m_addIndex);
    m_addIndex = 0;
} // slotKbdTimedOut


void Shortcuts::activateStation (int i)
{
    if (! sendActivateStation(i - 1))
        sendActivateStation( (i + 9) % 10);
} // activateStation


bool Shortcuts::connectI (Interface *i)
{
    bool a = IRadioClient::connectI (i);
    bool b = ITimeControlClient::connectI (i);
    bool c = IRadioDevicePoolClient::connectI (i);
    bool d = PluginBase::connectI(i);
    bool e = ISoundStreamClient::connectI(i);
    return a || b || c || d || e;
} // connectI


bool Shortcuts::disconnectI (Interface *i)
{
    bool a = IRadioClient::disconnectI (i);
    bool b = ITimeControlClient::disconnectI (i);
    bool c = IRadioDevicePoolClient::disconnectI (i);
    bool d = PluginBase::disconnectI(i);
    bool e = ISoundStreamClient::disconnectI(i);
    return a || b || c || d || e;
} // disconnectI



void   Shortcuts::saveState (KConfigGroup &c) const
{
    PluginBase::saveState(c);

    m_stdActions    ->collection()->writeSettings(&c);
    m_stationActions->collection()->writeSettings(&c);
} // saveState



void   Shortcuts::restoreState (const KConfigGroup &c)
{
    PluginBase::restoreState(c);

    m_stdActions    ->collection()->readSettings(const_cast<KConfigGroup*>(&c));
    m_stationActions->collection()->readSettings(const_cast<KConfigGroup*>(&c));
} // restoreState



ConfigPageInfo Shortcuts::createConfigurationPage()
{
    ShortcutsConfiguration *tmp = new ShortcutsConfiguration();
    QObject::connect(tmp, &QObject::destroyed, this, &Shortcuts::slotConfigPageDestroyed);
    m_ShortcutsEditors.append(tmp);
    updateShortcutsEditor(tmp);

    return ConfigPageInfo (tmp,
                           i18n("Shortcuts"),
                           i18n("Shortcuts Plugin"),
                           "preferences-desktop-keyboard"
                          );
} // createConfigurationPage



void Shortcuts::slotConfigPageDestroyed(QObject *o)
{
    // IMPORTANT: if this slot is called, the descructor of
    // ShortcutsConfiguration is already finished! Thus the object
    // is officially no ShortcutsConfiguration any more, dynamic_cast would fail.
    // But we need only the pointer here. That's ok.
    ShortcutsConfiguration *scc = static_cast<ShortcutsConfiguration*>(o);
    if (o && m_ShortcutsEditors.contains(scc)) {
        m_ShortcutsEditors.removeAll(scc);
    }
} // slotConfigPageDestroyed



void Shortcuts::updateShortcutsEditor(ShortcutsConfiguration *c)
{
    if (c) {
        c->editor()->clearCollections();
        c->editor()->addCollection(m_stdActions    ->collection(), "KRadio5");
        c->editor()->addCollection(m_stationActions->collection(), "KRadio5");
//         qDebug() << "number of standard shortcuts: " << m_stdActions    ->collection()->count();
//         qDebug() << "number of station shortcuts:  " << m_stationActions->collection()->count();
//         for (const auto & x : m_stdActions->collection()->actions()) {
//             qDebug() << "Shortcut: " << x->text() << " = " << x->shortcut().toString();
//         }
//         for (const auto & x : m_stationActions->collection()->actions()) {
//             qDebug() << "Shortcut: " << x->text() << " = " << x->shortcut().toString();
//         }
    }
} // updateShortcutsEditor


void Shortcuts::updateShortcutsEditors()
{
    ShortcutsConfiguration *sce = nullptr;
    foreach(sce, m_ShortcutsEditors) {
        updateShortcutsEditor(sce);
    }
} // updateShortcutsEditors



void Shortcuts::slotActionTriggered(QAction *a)
{
    if (!a) {
        return;
    }
    int action = a->data().toInt();

    SoundStreamID streamSinkID = queryCurrentSoundStreamSinkID();

    bool        q      = false;
    SoundFormat sf;
    ISeekRadio *seeker = NULL;

    int digit = -1;
    switch (action) {
        case SHORTCUT_DIGIT_0 :
            digit = 0;
            break;
        case SHORTCUT_DIGIT_1 :
            digit = 1;
            break;
        case SHORTCUT_DIGIT_2 :
            digit = 2;
            break;
        case SHORTCUT_DIGIT_3 :
            digit = 3;
            break;
        case SHORTCUT_DIGIT_4 :
            digit = 4;
            break;
        case SHORTCUT_DIGIT_5 :
            digit = 5;
            break;
        case SHORTCUT_DIGIT_6 :
            digit = 6;
            break;
        case SHORTCUT_DIGIT_7 :
            digit = 7;
            break;
        case SHORTCUT_DIGIT_8 :
            digit = 8;
            break;
        case SHORTCUT_DIGIT_9 :
            digit = 9;
            break;
        case SHORTCUT_POWER_ON :
            if (!queryIsPowerOn()) {
                sendPowerOn();
            }
            break;
        case SHORTCUT_POWER_OFF :
            if (queryIsPowerOn()) {
                sendPowerOff();
            }
            break;
        case SHORTCUT_PAUSE :
            if (queryIsPowerOn()) {
                bool paused = false;
                queryIsPlaybackPaused(streamSinkID, paused);
                if (paused) {
                    sendResumePlayback(streamSinkID);
                } else {
                    sendPausePlayback(streamSinkID);
                }
            }
            break;
        case SHORTCUT_RECORD_START :
            queryIsRecordingRunning(streamSinkID, q = false, sf);
            if (!q) {
                sendStartRecording(streamSinkID);
            }
            break;
        case SHORTCUT_RECORD_STOP :
            queryIsRecordingRunning(streamSinkID, q = false, sf);
            if (q) {
                sendStopRecording(streamSinkID);
            }
            break;
        case SHORTCUT_VOLUME_INC :
            if (queryIsPowerOn()) {
                float oldVolume = 0;
                queryPlaybackVolume(streamSinkID, oldVolume);
                sendPlaybackVolume (streamSinkID, oldVolume + 1.0/32.0);
            }
            break;
        case SHORTCUT_VOLUME_DEC :
            if (queryIsPowerOn()) {
                float oldVolume = 0;
                queryPlaybackVolume(streamSinkID, oldVolume);
                sendPlaybackVolume (streamSinkID, oldVolume - 1.0/32.0);
            }
            break;
        case SHORTCUT_FREQ_INC :
            if (queryIsPowerOn()) {
                float oldfreq = queryFrequency();
                float freqinc = queryScanStep();
                sendFrequency(oldfreq + freqinc);
            }
            break;
        case SHORTCUT_FREQ_DEC :
            if (queryIsPowerOn()) {
                float oldfreq = queryFrequency();
                float freqinc = queryScanStep();
                sendFrequency(oldfreq - freqinc);
            }
            break;
        case SHORTCUT_STATION_NEXT :
            if (queryIsPowerOn()) {
                int k = queryCurrentStationIdx() + 1;
                if (k >= queryStations().count())
                    k = 0;
                sendActivateStation(k);
            }
            break;
        case SHORTCUT_STATION_PREV :
            if (queryIsPowerOn()) {
                int k = queryCurrentStationIdx() - 1;
                if (k < 0)
                    k = queryStations().count() - 1;
                sendActivateStation(k);
            }
            break;
        case SHORTCUT_SEARCH_NEXT :
            if (queryIsPowerOn()) {
                seeker = dynamic_cast<ISeekRadio*> (queryActiveDevice());
                if (seeker) {
                    seeker->startSeekUp();
                }
            }
            break;
        case SHORTCUT_SEARCH_PREV :
            if (queryIsPowerOn()) {
                seeker = dynamic_cast<ISeekRadio*> (queryActiveDevice());
                if (seeker) {
                    seeker->startSeekDown();
                }
            }
            break;
        case SHORTCUT_SLEEP :
            if (queryIsPowerOn()) {
                sendStartCountdown();
            }
            break;
        case SHORTCUT_APPLICATION_QUIT :
            qApp->quit();
            break;
        default:
            break;
    }

    if (digit >= 0) {
        if (m_addIndex || digit == 0) {
            activateStation(m_addIndex * 10 + digit);
            m_kbdTimer->stop();
            m_addIndex = 0;
        } else {
            m_addIndex = digit;
            m_kbdTimer->setSingleShot(true);
            m_kbdTimer->start(250);
        }
    }
} // slotActionTriggered




void Shortcuts::noticePluginsAdded(const PluginList &l)
{
    PluginBase *p = NULL;
    foreach(p, l) {
        WidgetPluginBase *w = dynamic_cast<WidgetPluginBase*>(p);
        if (w) {
            QWidget *widget = w->getWidget();
            m_stdCollection    ->addAssociatedWidget(widget);
            m_stationCollection->addAssociatedWidget(widget);
        } // if plugin is widget

    } // foreach plugin
} // noticePluginsAdded



void Shortcuts::slotStationTriggered(QAction *a)
{
    if (!a) {
        return;
    }
    QString             StationID = a->data().toString();
    const StationList  &stations  = queryStations();
    const RadioStation &rs        = stations.stationWithID(StationID);
    const RadioStation &crs       = queryCurrentStation();
    QString rs_id  =  rs.stationID();
    QString crs_id = crs.stationID();
    sendActivateStation(rs);
/*    logDebug(QString("Shortcuts::slotStationTriggered: rs: %1, %2").arg(rs.longName(), rs.stationID()));
    logDebug(QString("Shortcuts::slotStationTriggered: crs: %1, %2").arg(crs.longName(), crs.stationID()));*/
    if (rs_id == crs_id) {
        if (queryIsPowerOn()) {
/*            logDebug("Shortcuts::slotStationTriggered: eqID, sendPowerOff()");*/
            sendPowerOff();
        } else {
/*            logDebug("Shortcuts::slotStationTriggered: eqID, sendPowerOn()");*/
            sendPowerOn();
        }
    } else {
        if (!queryIsPowerOn()) {
/*            logDebug("Shortcuts::slotStationTriggered: neqID, sendPowerOn()");*/
            sendPowerOn();
        }
    }
} // slotStationTriggered



bool Shortcuts::noticeStationsChanged(const StationList &sl)
{
    QMap<QString, QAction *> oldActions;
    QAction *_a = NULL;
    foreach(_a, m_stationCollection->actions()) {
        QAction *a = dynamic_cast<QAction*>(_a);
        if (a) {
            m_stationCollection->takeAction(a);
            QString strdata = a->data().toString();
//             logDebug("Action Data: " + strdata + "\n");
            oldActions.insert(strdata, a);
        }
    }
    m_stationCollection->clear();

    int idx = 1;
    for (StationList::const_iterator it = sl.begin(); it != sl.end(); ++it, ++idx) {
        const RadioStation *s  = *it;
        const QString      &id = s->stationID();

        QAction *a = NULL;
        if (oldActions.contains(id)) {
            a = oldActions[id];
            m_stationActions->addAction(id, a);
            oldActions.remove(id);
        } else {
            QAction *_a = m_stationActions->addAction(id);
            KGlobalAccel::setGlobalShortcut(_a, QKeySequence());
            a = _a;
        }
        a->setData(id);
        a->setText(QString().sprintf("%02i) ", idx) + s->name());
        if (s->iconName().length()) {
            a->setIcon(QIcon::fromTheme(s->iconName()));
        }
    }
    qDeleteAll(oldActions);
    updateShortcutsEditors();

    return true;
} // noticeStationsChanged

