/***************************************************************************
                          radio-configuration.cpp  -  description
                             -------------------
    begin                : Son Aug 3 2003
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

#include "radiostation.h"
#include "stationlist.h"
#include "pluginbase.h"
#include "radiodevice_interfaces.h"
#include "standardscandialog.h"
#include "radiostation-listview.h"
#include "radiostation-config.h"
#include "errorlog_interfaces.h"

#include "radio-configuration.h"

#include <math.h>

#include <QListWidget>
#include <QDateTimeEdit>
#include <QLineEdit>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QMenu>
#include <QStackedWidget>

#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <kurllabel.h>
#include <kurlrequester.h>
#include <klocalizedstring.h>
#include <kdemacros.h>
#include <ktoolinvocation.h>
#include <kglobal.h>
#include <kicon.h>

RadioConfiguration::RadioConfiguration (QWidget *parent, const IErrorLogClient &logger)
    : QWidget(parent),
      Ui_RadioConfigurationUI(),
      m_ignoreChanges(false),
      m_devicePopup(NULL),
      m_logger(logger),
      m_dirty(true)
{
    setupUi(this);

    QString defaultPresetDir = KGlobal::dirs()->saveLocation("data", "kradio4");

    editPresetFile->setStartDir(defaultPresetDir);

    // icon settings does not work any more in .ui files in KDE4, don't know why/how
    buttonNewStation      ->setIcon(KIcon("document-new"));
    buttonDeleteStation   ->setIcon(KIcon("edit-delete"));
    buttonStationUp       ->setIcon(KIcon("arrow-up"));
    buttonStationDown     ->setIcon(KIcon("arrow-down"));
    buttonSearchStations  ->setIcon(KIcon("edit-find"));
    buttonLoadPresets     ->setIcon(KIcon("document-open"));
    buttonStorePresets    ->setIcon(KIcon("document-save-as"));
    buttonSelectPixmapFile->setIcon(KIcon("document-open"));
    editVolumePreset      ->setSpecialValueText(i18n("unchanged"));

    comboStereoMode->clear();
    comboStereoMode->addItem(i18nc("Sound mode", "<do not care>"),        (int)STATION_STEREO_DONTCARE);
    comboStereoMode->addItem(i18nc("Sound mode, monophonic", "Mono"),     (int)STATION_STEREO_OFF);
    comboStereoMode->addItem(i18nc("Sound mode, stereophonic", "Stereo"), (int)STATION_STEREO_ON);

    m_loadPopup = new QMenu(buttonLoadPresets);
    m_loadPopup->addAction(KIcon("document-open"), i18n("Load and replace presets"), this, SLOT(slotLoadPresets()));
    m_loadPopup->addAction(KIcon("list-add"),      i18n("Load and add presets"),     this, SLOT(slotAddPresets ()));
    buttonLoadPresets->setMenu(m_loadPopup);

    QObject::connect(listStations, SIGNAL(sigCurrentStationChanged(int)),
                     this, SLOT(slotStationSelectionChanged(int)));
    QObject::connect(buttonSelectPixmapFile, SIGNAL(clicked()),
                     this, SLOT(slotSelectPixmap()));
    QObject::connect(buttonNewStation, SIGNAL(clicked()),
                     this, SLOT(slotNewStation()));
    QObject::connect(buttonDeleteStation, SIGNAL(clicked()),
                     this, SLOT(slotDeleteStation()));
    QObject::connect(editPixmapFile, SIGNAL(textChanged(const QString &)),
                     this, SLOT(slotPixmapChanged(const QString &)));
    QObject::connect(editStationName, SIGNAL(textChanged(const QString &)),
                     this, SLOT(slotStationNameChanged(const QString &)));
    QObject::connect(editStationShortName, SIGNAL(textChanged(const QString &)),
                     this, SLOT(slotStationShortNameChanged(const QString &)));
    QObject::connect(editVolumePreset, SIGNAL(valueChanged(int)),
                     this, SLOT(slotVolumePresetChanged(int)));
    QObject::connect(comboStereoMode, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(slotStereoModeChanged(int)));
    QObject::connect(buttonStationUp, SIGNAL(clicked()),
                     this, SLOT(slotStationUp()));
    QObject::connect(buttonStationDown, SIGNAL(clicked()),
                     this, SLOT(slotStationDown()));
    QObject::connect(listStations, SIGNAL(sigStationActivated(int)),
                     this, SLOT(slotActivateStation( int )));
    QObject::connect(buttonStorePresets, SIGNAL(clicked()),
                     this, SLOT(slotStorePresets()));
    QObject::connect(buttonLastChangeNow, SIGNAL(clicked()),
                     this, SLOT(slotLastChangeNow()));

    connect(editMaintainer, SIGNAL(textChanged(const QString &)),       SLOT(slotSetDirty()));
    connect(editLastChange, SIGNAL(dateTimeChanged(const QDateTime &)), SLOT(slotSetDirty()));
    connect(editCountry,    SIGNAL(textChanged(const QString &)),       SLOT(slotSetDirty()));
    connect(editCity,       SIGNAL(textChanged(const QString &)),       SLOT(slotSetDirty()));
    connect(editMedia,      SIGNAL(textChanged(const QString &)),       SLOT(slotSetDirty()));
    connect(editComment,    SIGNAL(textChanged(const QString &)),       SLOT(slotSetDirty()));
    connect(editPresetFile, SIGNAL(textChanged(const QString &)),       SLOT(slotSetDirty()));

    mailLabel->setText("emw-kradio-presets@nocabal.de");
    mailLabel->setUrl ("emw-kradio-presets@nocabal.de");
    QObject::connect(mailLabel, SIGNAL(leftClickedUrl(const QString &)),
                     this, SLOT(slotSendPresetsByMail(const QString &)));

    m_devicePopup = new QMenu(buttonSearchStations);
    buttonSearchStations->setMenu(m_devicePopup);
    QObject::connect(m_devicePopup, SIGNAL(triggered(QAction*)),
                     this, SLOT(slotSearchStations(QAction*)));



    m_stationTypeMenu = new QMenu(this);

    const QList<RadioStation *> &classes = RadioStation::getStationClasses();
    const RadioStation *st = NULL;
    foreach (st, classes) {
        if (st && st->isClassUserVisible()) {
            QString classname = st->getClassName();
            QString classdesc = st->getClassDescription();
            QAction *a = m_stationTypeMenu->addAction(classdesc);
            a->setData(classname);
        }
    }
    QObject::connect(m_stationTypeMenu, SIGNAL(triggered(QAction *)), this, SLOT(slotNewStation(QAction *)));
    buttonNewStation->setMenu(m_stationTypeMenu);
}


RadioConfiguration::~RadioConfiguration ()
{
}


bool RadioConfiguration::connectI (Interface *i)
{
    bool a = IRadioClient::connectI(i);
    bool b = IRadioDevicePoolClient::connectI(i);

    return a || b;
}

bool RadioConfiguration::disconnectI (Interface *i)
{
    bool a = IRadioClient::disconnectI(i);
    bool b = IRadioDevicePoolClient::disconnectI(i);

    return a || b;
}

// IRadioDevicePoolClient

bool RadioConfiguration::noticeDevicesChanged(const QList<IRadioDevice*> &l)
{
    m_devices.clear();
    m_devicePopup->clear();
    int id = 0;
    foreach (IRadioDevice *d, l) {
        if (dynamic_cast<ISeekRadio*>(d)) {
            QAction *a = m_devicePopup->addAction(d->getDescription());
            a->setData(id++);
            //m_devicePopup->insertItem(d->getDescription(), id++);
            m_devices.append(d);
        }
    }
    return true;
}


bool RadioConfiguration::noticeDeviceDescriptionChanged(const QString &)
{
    noticeDevicesChanged(queryDevices());
    return true;
}


// IRadioClient

bool RadioConfiguration::noticeStationsChanged(const StationList &sl)
{
    m_ignoreChanges = true;

    QString old_id = listStations->currentStationID();

    m_stations = sl;

    listStations->setStations(sl);

    listStations->setCurrentStation(m_stations.idxWithID(old_id));
    listStations->scrollTo(listStations->currentIndex());


    StationListMetaData &info = m_stations.metaData();

    editMaintainer->setText(info.maintainer);
    editLastChange->setDateTime(info.lastChange);
    editCountry->setText(info.country);
    editCity->setText(info.city);
    editMedia->setText(info.media);
    editComment->setText(info.comment);

    m_ignoreChanges = false;

    slotStationSelectionChanged(listStations->currentStationIndex());

    return true;
}


bool RadioConfiguration::noticePresetFileChanged(const QString &f)
{
    m_ignoreChanges = true;
    editPresetFile->setUrl(f);
    m_ignoreChanges = false;
    return true;
}


void RadioConfiguration::slotStationSelectionChanged(int idx)
{
    RadioStation *s = NULL;

    if (idx >= 0 && idx < m_stations.count()) {
        s = &m_stations.at(idx);
    }

    editStationName       ->setDisabled(!s);
    labelStationName      ->setDisabled(!s);
    editPixmapFile        ->setDisabled(!s);
    labelPixmapFile       ->setDisabled(!s);
    editStationShortName  ->setDisabled(!s);
    labelStationShortName ->setDisabled(!s);
    editVolumePreset      ->setDisabled(!s);
    labelVolumePreset     ->setDisabled(!s);
    buttonSelectPixmapFile->setDisabled(!s);
    buttonDeleteStation   ->setDisabled(!s);

    buttonStationUp       ->setDisabled(!s || idx == 0);
    buttonStationDown     ->setDisabled(!s || idx == m_stations.count()-1);

    if (m_ignoreChanges) return;
    m_ignoreChanges = true;

    editStationName       ->setText  (s ? s->name() : QString());
    editStationShortName  ->setText  (s ? s->shortName() : QString());
    editPixmapFile        ->setText  (s ? s->iconName() : QString());
    editVolumePreset      ->setValue (s ? (int)rint(s->initialVolume()*100) : -1);
    int stModeIdx         = comboStereoMode->findData(s ? (int)s->stereoMode() : (int)STATION_STEREO_DONTCARE);
    comboStereoMode       ->setCurrentIndex(stModeIdx >= 0 ? stModeIdx : comboStereoMode->findData((int)STATION_STEREO_DONTCARE));

    stackStationEdit->setDisabled(!s);
    if (s) {
        QMap<QString, RadioStationConfig*>::const_iterator it_c = m_stationEditors.constFind(s->getClassName());
        RadioStationConfig *c = it_c != m_stationEditors.constEnd() ? *it_c : NULL;
        if (!c) {
            c = s->createEditor();
            if (c) {
                c->setParent(this);
                c->move(QPoint(0,0));
                c->show();
                QObject::connect(c, SIGNAL(changed(RadioStationConfig*)),
                                 this, SLOT(slotStationEditorChanged(RadioStationConfig*)));
                m_stationEditors.insert(s->getClassName(), c);
                stackStationEdit->addWidget(c);
            }
        }
        if (c) {
            c->setStationData(*s);
            stackStationEdit->setCurrentWidget(c);
        }
    }

    m_ignoreChanges = false;
}


void RadioConfiguration::slotNewStation()
{
    slotSetDirty();
    const RadioStation *st    = &queryCurrentStation();
    createNewStation(st);
}


void RadioConfiguration::slotNewStation(QAction *a)
{
    slotSetDirty();

    QString classname = a->data().toString();
    const RadioStation *st = RadioStation::getStationClass(classname);
    if (st) {
        createNewStation(st);
    }
}


void RadioConfiguration::createNewStation(const RadioStation *rs_template)
{
    RadioStation   *newst      = rs_template->copyNewID();
    int             currentIdx = listStations->currentStationIndex();
    int             n          = m_stations.count();
    if (currentIdx < 0) {
        currentIdx = n - 1;
    }

    m_stations.addStation(*newst);
    if (m_stations.count() > n) {
        m_stations.moveStation(n, currentIdx + 1);

        m_ignoreChanges = true;
        listStations->appendStation(*newst);
        for (int i = currentIdx + 1; i <= n; ++i) {
            listStations->setStation(i, m_stations.at(i));
        }
        listStations->setCurrentStation(currentIdx + 1);
        listStations->scrollTo(listStations->currentIndex());
        m_ignoreChanges = false;
        slotStationSelectionChanged(listStations->currentStationIndex());

    }
    delete newst;
}


void RadioConfiguration::slotDeleteStation()
{
    int idx = listStations->currentStationIndex();

    if (idx >= 0 && idx < m_stations.count()) {
        slotSetDirty();
        m_stations.removeStationAt(idx);
        listStations->removeStation(idx);
    }
}


void RadioConfiguration::slotStationEditorChanged(RadioStationConfig *c)
{
    if (!c) return;
    if (m_ignoreChanges) return;


    int idx = listStations->currentStationIndex();
    if (idx >= 0 && idx < m_stations.count()) {
        slotSetDirty();
        RadioStation &st = m_stations.at(idx);

        m_ignoreChanges = true;
        bool o = listStations->signalsBlocked();
        listStations->blockSignals(true);

        c->storeStationData(st);
        listStations->setStation(idx, st);

        listStations->blockSignals(o);
        m_ignoreChanges = false;
    }
}


void RadioConfiguration::slotStationNameChanged( const QString & s)
{
    if (m_ignoreChanges) return;

    int idx = listStations->currentStationIndex();
    if (idx >= 0 && idx < m_stations.count()) {
        slotSetDirty();
        RadioStation &st = m_stations.at(idx);
        st.setName(s);
        m_ignoreChanges = true;
        bool o = listStations->signalsBlocked();
        listStations->blockSignals(true);
        listStations->setStation(idx, st);
        listStations->blockSignals(o);
        m_ignoreChanges = false;
    }
}


void RadioConfiguration::slotStationShortNameChanged( const QString & sn)
{
    if (m_ignoreChanges) return;

    int idx = listStations->currentStationIndex();
    if (idx >= 0 && idx < m_stations.count()) {
        slotSetDirty();
        RadioStation &st = m_stations.at(idx);
        st.setShortName(sn);
        m_ignoreChanges = true;
        bool o = listStations->signalsBlocked();
        listStations->blockSignals(true);
        listStations->setStation(idx, st);
        listStations->blockSignals(o);
        m_ignoreChanges = false;
    }
}


void RadioConfiguration::slotSelectPixmap()
{
    KUrl url = KFileDialog::getImageOpenUrl(QString(), this,
                                            i18n("Image Selection"));
    if (!url.isEmpty()) {
        if (url.isLocalFile()) {
            editPixmapFile->setText(url.path());
        } else {
            m_logger.logWarning(i18n("ignoring non-local image"));
        }
    }
}


void RadioConfiguration::slotPixmapChanged( const QString &s )
{
    if (m_ignoreChanges) return;

    int idx = listStations->currentStationIndex();
    if (idx >= 0 && idx < m_stations.count()) {
        slotSetDirty();
        RadioStation &st = m_stations.at(idx);
        st.setIconName(s);
        m_ignoreChanges = true;
        bool o = listStations->signalsBlocked();
        listStations->blockSignals(true);
        listStations->setStation(idx, st);
        listStations->blockSignals(o);
        m_ignoreChanges = false;
    }
}


void RadioConfiguration::slotVolumePresetChanged(int v)
{
    int idx = listStations->currentStationIndex();
    if (idx >= 0 && idx < m_stations.count()) {
        slotSetDirty();
        RadioStation &s = m_stations.at(idx);
        s.setInitialVolume(0.01 * (double)v);
    }
}


void RadioConfiguration::slotStereoModeChanged(int mode_idx)
{
    if (mode_idx >= 0) {
        int mode = comboStereoMode->itemData(mode_idx).toInt();
        int st_idx = listStations->currentStationIndex();
        if (st_idx >= 0 && st_idx < m_stations.count()) {
            slotSetDirty();
            RadioStation &s = m_stations.at(st_idx);
            s.setStereoMode((StationStereoMode)mode);
        }
    }
}




void RadioConfiguration::slotStationUp()
{
    int idx = listStations->currentStationIndex();
    if (idx > 0 && idx < m_stations.count()) {
        slotSetDirty();
        StationList &sl = m_stations;

        sl.moveStation(idx-1, idx);

        m_ignoreChanges = true;
//         bool o = listStations->signalsBlocked();
//         listStations->blockSignals(true);
        listStations->setStation(idx-1, sl.at(idx-1));
        listStations->setStation(idx,   sl.at(idx));
        listStations->setCurrentStation(idx-1);
        listStations->scrollTo(listStations->currentIndex());
//         listStations->blockSignals(o);
        m_ignoreChanges = false;
    }
}


void RadioConfiguration::slotStationDown()
{
    int idx = listStations->currentStationIndex();
    if (idx >= 0 && idx < m_stations.count() - 1) {
        slotSetDirty();
        StationList &sl = m_stations;

        sl.moveStation(idx, idx+1);

        m_ignoreChanges = true;
//         bool o = listStations->signalsBlocked();
//         listStations->blockSignals(true);
        listStations->setStation(idx,   sl.at(idx));
        listStations->setStation(idx+1, sl.at(idx+1));
        listStations->setCurrentStation(idx+1);
        listStations->scrollTo(listStations->currentIndex());
//         listStations->blockSignals(o);
        m_ignoreChanges = false;
    }
}


void RadioConfiguration::slotActivateStation(int idx)
{
    if (idx >= 0 && idx < m_stations.count()) {
        sendActivateStation(m_stations.at(idx));
        sendPowerOn();
    }
}

void RadioConfiguration::slotLoadPresets()
{
    loadPresets(false);
}

void RadioConfiguration::slotAddPresets()
{
    loadPresets(true);
}

void RadioConfiguration::loadPresets(bool add)
{
    const KUrl::List urls = KFileDialog::getOpenUrls(KUrl(KStandardDirs::installPath("data") + "kradio4/presets"), "*.krp|" + i18n("KRadio Preset Files"), this, i18n("Select Preset File"));

    if (!urls.isEmpty()) {
        slotSetDirty();
        StationList sl_all;
        if (add) {
            sl_all = m_stations;
        }
        KUrl url;
        foreach (url, urls) {
            StationList sl;
            if (sl.readXML(url, m_logger)) {
                sl_all.addStations(sl);
            }
        }
        noticeStationsChanged(sl_all);
    }
}


void RadioConfiguration::slotStorePresets()
{
    const KUrl url = KFileDialog::getSaveUrl(KUrl(), "*.krp|" + i18n("KRadio Preset Files"), this, i18n("Save Preset File"), KFileDialog::ConfirmOverwrite);

    if (url.isValid()) {
        editPresetFile->setUrl(url.url());
        m_stations.writeXML(url, m_logger);
    }
}


void RadioConfiguration::slotLastChangeNow()
{
    slotSetDirty();
    editLastChange->setDateTime(QDateTime::currentDateTime());
}


void RadioConfiguration::slotSendPresetsByMail( const QString &url )
{
    QString preset_file = queryPresetFile();

    QString country = m_stations.metaData().country;
    QString city    = m_stations.metaData().city;
    QString location = city + "/" + country;

    KToolInvocation::invokeMailer(url, QString(), QString(),
                                  "station preset file for " + location,
                                  QString(), QString(),
                                  QStringList() << preset_file);
}


void RadioConfiguration::slotSearchStations(QAction *a)
{
    int idev = a ? a->data().toInt() : -1;
    if (idev >= 0 && idev < m_devices.count()) {
        IRadioDevice *dev = m_devices.at(idev);

        StandardScanDialog *x = new StandardScanDialog(NULL);
        x->connectI(dev);                                        // connect device
        x->connectI(IRadioDevicePoolClient::iConnections.at(0)); // connect radio to get verbous station information
        sendActiveDevice(dev);
        x->show();
        x->start();
        if (x->exec() == QDialog::Accepted) {
            slotSetDirty();
            const StationList &found = x->getStations();
            m_logger.logDebug(i18np("found %1 new station", "found %1 new stations", found.count()));
            m_stations.merge(found);
            m_logger.logDebug(i18np("have now %1 station", "have now %1 stations", m_stations.count()));
            noticeStationsChanged(m_stations);
        }
        delete x;
//        logDebug("scan finished");
    }
//    logDebug("scan finished completely");
}


void RadioConfiguration::slotOK()
{
    if (m_dirty) {
        StationListMetaData &i = m_stations.metaData();

        i.maintainer = editMaintainer->text();
        i.lastChange = editLastChange->dateTime();
        i.country    = editCountry->text();
        i.city       = editCity->text();
        i.media      = editMedia->text();
        i.comment    = editComment->text();

        sendStations(m_stations);
        sendPresetFile(editPresetFile->url().pathOrUrl());
        m_dirty = false;
    }
}

void RadioConfiguration::slotCancel()
{
    if (m_dirty) {
        noticeStationsChanged(queryStations());
        noticePresetFileChanged(queryPresetFile());
        m_dirty = false;
    }
}


void RadioConfiguration::slotSetDirty()
{
    if (!m_ignoreChanges) {
        m_dirty = true;
    }
}


#include "radio-configuration.moc"
