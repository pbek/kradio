/***************************************************************************
                          radio-configuration.cpp  -  description
                             -------------------
    begin                : Son Aug 3 2003
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

#include "radio-configuration.h"
#include "radiostation.h"
#include "stationlist.h"
#include "plugins.h"
#include "radiodevice_interfaces.h"
#include "standardscandialog.h"
#include "radiostation-config.h"

#include <qlistbox.h>
#include <klistbox.h>
#include <qdatetimeedit.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qtoolbutton.h>
#include <qwidgetstack.h>

#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <kurllabel.h>
#include <qregexp.h>
#include <krun.h>

RadioConfiguration::RadioConfiguration (QWidget *parent)
	: RadioConfigurationUI(parent),
	  ignoreChanges(false),
	  devicePopup(NULL)
{
	QObject::connect(listStations, SIGNAL(highlighted(int)),
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
	QObject::connect(buttonStationUp, SIGNAL(clicked()),
					 this, SLOT(slotStationUp()));
	QObject::connect(buttonStationDown, SIGNAL(clicked()),
					 this, SLOT(slotStationDown()));
	QObject::connect(listStations, SIGNAL(selected(int)),
					 this, SLOT(slotActivateStation( int )));
	QObject::connect(buttonLoadPresets, SIGNAL(clicked()),
					 this, SLOT(slotLoadPresets()));
	QObject::connect(buttonStorePresets, SIGNAL(clicked()),
					 this, SLOT(slotStorePresets()));
	QObject::connect(buttonLastChangeNow, SIGNAL(clicked()),
					 this, SLOT(slotLastChangeNow()));
					 
    mailLabel->setText("mailto:witte-presets@kawo1.rwth-aachen.de");
    mailLabel->setURL ("mailto:witte-presets@kawo1.rwth-aachen.de");
    QObject::connect(mailLabel, SIGNAL(leftClickedURL(const QString &)),
	                 this, SLOT(slotSendPresetsByMail(const QString &)));

	QObject::connect(buttonSearchStations, SIGNAL(clicked()),
					 this, SLOT(slotSearchStations0()));

}


RadioConfiguration::~RadioConfiguration ()
{
}


bool RadioConfiguration::connect (Interface *i)
{
	bool a = IRadioClient::connect(i);
	bool b = IRadioDevicePoolClient::connect(i);

	return a || b;
}

bool RadioConfiguration::disconnect (Interface *i)
{
	bool a = IRadioClient::disconnect(i);
	bool b = IRadioDevicePoolClient::disconnect(i);

	return a || b;
}

// IRadioDevicePoolClient

bool RadioConfiguration::noticeDevicesChanged(const QPtrList<IRadioDevice> &l)
{
	QPtrListIterator<IRadioDevice> it(l);
	QPopupMenu *oldPopup = devicePopup;
	devicePopup = new QPopupMenu();
	devices.clear();
	int id = 0;
	for (; it.current(); ++it) {
		IRadioDevice *d = it.current();
		if (dynamic_cast<ISeekRadio*>(d)) {
			devicePopup->insertItem(d->getDescription(), id++);
			devices.append(d);
		}
	}
	QObject::connect(devicePopup, SIGNAL(activated(int)),
					 this, SLOT(slotSearchStations(int)));
	buttonSearchStations->setPopup(devicePopup);
	if (oldPopup) delete oldPopup;
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
	m_stations = sl;
	listStations->clear();

	for (RawStationList::Iterator it(m_stations.all()); it.current(); ++it) {
		RadioStation *s = it.current();
		listStations->insertItem(s->iconName(), s->longName());
    }

    StationListMetaData &info = m_stations.metaData();

    editMaintainer->setText(info.maintainer);
    editLastChange->setDateTime(info.lastChange);
    editCountry->setText(info.country);
    editCity->setText(info.city);
    editMedia->setText(info.media);
    editComment->setText(info.comment);

    slotStationSelectionChanged(listStations->currentItem());
    
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

    if (ignoreChanges) return;
    ignoreChanges = true;
    
	editStationName       ->setText  (s ? s->name() : QString::null);
	editStationShortName  ->setText  (s ? s->shortName() : QString::null);
	editPixmapFile        ->setText  (s ? s->iconName() : QString::null);
	editVolumePreset      ->setValue (s ? (int)rint(s->initialVolume()*100) : -1);
	pixmapStation         ->setPixmap(s ? QPixmap(s->iconName()) : QPixmap());

	stackStationEdit->setDisabled(!s);
	if (s) {
		RadioStationConfig *c = stationEditors.find(s->getClassName());
        if (!c) {
			c = s->createEditor();
			if (c) {
				c->reparent(this, QPoint(0,0), true);
				QObject::connect(c, SIGNAL(changed(RadioStationConfig*)),
			                     this, SLOT(slotStationEditorChanged(RadioStationConfig*)));
			    stationEditors.insert(s->getClassName(), c);
			    stackStationEdit->addWidget(c);
			}
        }
        if (c) {
			c->setStationData(*s);
			stackStationEdit->raiseWidget(c);			
		}		
	}

    ignoreChanges = false;
}


void RadioConfiguration::slotNewStation()
{
	const RadioStation *st = &queryCurrentStation();
	int n = m_stations.count();
	m_stations.all().append(st);
	if (m_stations.count() == n) {
		st = st->getStationClass();
		m_stations.all().append(st);
	}
	if (m_stations.count() > n) {
		listStations->insertItem (st->iconName(), st->longName());
		listStations->setCurrentItem (listStations->count()-1);
	}
}


void RadioConfiguration::slotDeleteStation()
{
    int idx = listStations->currentItem();

    if (idx >= 0 && idx < m_stations.count()) {
		m_stations.all().remove(idx);
		listStations->removeItem(idx);
    }
}

void RadioConfiguration::slotStationEditorChanged(RadioStationConfig *c)
{
	if (!c) return;
	if (ignoreChanges) return;
	
    int idx = listStations->currentItem();
    if (idx >= 0 && idx < m_stations.count()) {
		RadioStation &st = m_stations.at(idx);
		c->storeStationData(st);
		ignoreChanges = true;
		bool o = listStations->signalsBlocked();
		listStations->blockSignals(true);
		listStations->changeItem(st.iconName(), st.longName(), idx);
		listStations->blockSignals(o);
		ignoreChanges = false;
	}
}


void RadioConfiguration::slotStationNameChanged( const QString & s)
{
	if (ignoreChanges) return;

    int idx = listStations->currentItem();
    if (idx >= 0 && idx < m_stations.count()) {
		RadioStation &st = m_stations.at(idx);
		st.setName(s);
		ignoreChanges = true;
		bool o = listStations->signalsBlocked();
		listStations->blockSignals(true);
		listStations->changeItem(st.iconName(), st.longName(), idx);
		listStations->blockSignals(o);
		ignoreChanges = false;
    }
}


void RadioConfiguration::slotStationShortNameChanged( const QString & sn)
{
	if (ignoreChanges) return;

    int idx = listStations->currentItem();
    if (idx >= 0 && idx < m_stations.count()) {
		RadioStation &st = m_stations.at(idx);
		st.setShortName(sn);
		ignoreChanges = true;
		bool o = listStations->signalsBlocked();
		listStations->blockSignals(true);
		listStations->changeItem(st.iconName(), st.longName(), idx);
		listStations->blockSignals(o);
		ignoreChanges = false;
    }
}


void RadioConfiguration::slotSelectPixmap()
{
    KFileDialog fd(QString::null,
				   "*.gif *.png *.jpg *.xpm|" + i18n("Images") + "(*.gif, *.png, *.jpg, *.xpm)",
				   this, i18n("Pixmap Selection"), true);
    fd.setMode(KFile::File | KFile::ExistingOnly);
    fd.setCaption (i18n("Select Station Pixmap"));

    if (fd.exec() == QDialog::Accepted) {
		QString filename = fd.selectedFile();
		editPixmapFile->setText(filename);
    }
}


void RadioConfiguration::slotPixmapChanged( const QString &s )
{
	if (ignoreChanges) return;

    int idx = listStations->currentItem();
    if (idx >= 0 && idx < m_stations.count()) {
		RadioStation &st = m_stations.at(idx);
		st.setIconName(s);
		ignoreChanges = true;
		pixmapStation->setPixmap(QPixmap(s));
		bool o = listStations->signalsBlocked();
		listStations->blockSignals(true);
		listStations->changeItem(st.iconName(), st.longName(), idx);
		listStations->blockSignals(o);
		ignoreChanges = false;
    }
}


void RadioConfiguration::slotVolumePresetChanged(int v)
{
    int idx = listStations->currentItem();
    if (idx >= 0 && idx < m_stations.count()) {
		RadioStation &s = m_stations.at(idx);
		s.setInitialVolume(0.01 * (double)v);
    }
}



void RadioConfiguration::slotStationUp()
{
    int idx = listStations->currentItem();
    if (idx > 0 && idx < m_stations.count()) {
		RawStationList &sl = m_stations.all();
		
		RadioStation *st = sl.take(idx-1);
        sl.insert(idx, st);
        delete st;
		
		ignoreChanges = true;
		bool o = listStations->signalsBlocked();
		listStations->blockSignals(true);
		listStations->changeItem(sl.at(idx-1)->iconName(), sl.at(idx-1)->longName(), idx-1);
		listStations->changeItem(sl.at(idx)  ->iconName(), sl.at(idx)  ->longName(), idx);
		listStations->setCurrentItem(idx-1);
		listStations->blockSignals(o);
		ignoreChanges = false;
    }                          
}


void RadioConfiguration::slotStationDown()
{
    int idx = listStations->currentItem();
    if (idx >= 0 && idx < m_stations.count() - 1) {
		RawStationList &sl = m_stations.all();

		RadioStation *st = sl.take(idx);
        sl.insert(idx+1, st);
        delete st;

		ignoreChanges = true;
		bool o = listStations->signalsBlocked();
		listStations->blockSignals(true);
		listStations->changeItem(sl.at(idx)  ->iconName(), sl.at(idx)  ->longName(), idx);
		listStations->changeItem(sl.at(idx+1)->iconName(), sl.at(idx+1)->longName(), idx+1);
		listStations->setCurrentItem(idx+1);
		listStations->blockSignals(o);
		ignoreChanges = false;
    }
}


void RadioConfiguration::slotActivateStation( int )
{
    int idx = listStations->currentItem();
    if (idx >= 0 && idx < m_stations.count()) {		
		sendActivateStation(m_stations.at(idx));
		sendPowerOn();
    }
}

void RadioConfiguration::slotLoadPresets()
{
    KFileDialog fd(locate("data", "kradio/presets/"),
		           "*.krp|" + i18n("KRadio Preset Files"),
		           this,
		           i18n("Preset File Selection"),
		           true);
    fd.setMode(KFile::File | KFile::ExistingOnly);
    fd.setCaption (i18n("Select Preset File"));

    if (fd.exec() == QDialog::Accepted) {
		StationList sl;
		if (sl.readXML(fd.selectedURL())) {
			noticeStationsChanged(sl);
		}
    }
}


void RadioConfiguration::slotStorePresets()
{
    KFileDialog fd("",
		           "*.krp|" + i18n("KRadio Preset Files"),
		           this,
		           i18n("Preset File Selection"),
		           true);
    fd.setMode(KFile::File);
    fd.setCaption (i18n("Store Preset File"));

    if (fd.exec() == QDialog::Accepted) {
		m_stations.writeXML(fd.selectedURL());
    }
}


void RadioConfiguration::slotLastChangeNow()
{
    editLastChange->setDateTime(QDateTime::currentDateTime());
}


static QString &urlEscapes(QString &s)
{
    s.replace(QRegExp("%"),  "%25");
    s.replace(QRegExp("\t"),  "%09");
    s.replace(QRegExp("\n"),  "%0A");
    s.replace(QRegExp("\n"),  "%0D");
    s.replace(QRegExp(" "),   "%20");
    s.replace(QRegExp("\\!"), "%21");
    s.replace(QRegExp("\""),  "%22");
    s.replace(QRegExp("#"),   "%23");
    s.replace(QRegExp("\\$"), "%24");
    s.replace(QRegExp("\\&"), "%26");
    s.replace(QRegExp("'"),   "%27");
    s.replace(QRegExp(","),   "%2C");
    s.replace(QRegExp(":"),   "%3A");
    s.replace(QRegExp(";"),   "%3B");
    s.replace(QRegExp("="),   "%3D");
    s.replace(QRegExp("\\?"), "%3F");
    return s;
}

void RadioConfiguration::slotSendPresetsByMail( const QString &url )
{
    QString presets = m_stations.writeXML();

    urlEscapes(presets);

    // documentation says, krun object deletes itself,
    // so we do not need to store the pointer

    QString country = m_stations.metaData().country;
    QString city    = m_stations.metaData().city;
    QString location = city + "/" + country;
    urlEscapes(location);

    QString cmd = url + "?subject=station preset file for " + location + "&body=";
       
    cmd += presets;
    new KRun (cmd);
}


void RadioConfiguration::slotSearchStations(int idev)
{
	if (idev >= 0 && (unsigned)idev < devices.count()) {
		IRadioDevice *dev = devices.at(idev);

		StandardScanDialog x(NULL);
		x.connect(dev);                                       // connect device
		x.connect(IRadioDevicePoolClient::connections.at(0)); // connect radio to get verbous station information
		sendActiveDevice(dev);
		x.show();
		x.start();
		if (x.exec() == QDialog::Accepted) {
			m_stations.merge(x.getStations());
			noticeStationsChanged(m_stations);
		}
	}
}


void RadioConfiguration::slotOK()
{
	StationListMetaData &i = m_stations.metaData();

	i.maintainer = editMaintainer->text();
	i.lastChange = editLastChange->dateTime();
	i.country    = editCountry->text();
	i.city       = editCity->text();
	i.media      = editMedia->text();
	i.comment    = editComment->text();
	
	sendStations(m_stations);
}

void RadioConfiguration::slotCancel()
{
	noticeStationsChanged(queryStations());
}

