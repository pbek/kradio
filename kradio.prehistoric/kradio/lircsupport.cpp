/***************************************************************************
                          lircsupport.cpp  -  description
                             -------------------
    begin                : Mon Feb 4 2002
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

#include "lircsupport.h"

#ifdef HAVE_LIRC_CLIENT
#include <lirc/lirc_client.h>
#endif

#include <qsocketnotifier.h>
#include <qtimer.h>
#include <kapplication.h>

#include "stationlist.h"
#include "radiodevice_interfaces.h"

#include <kaboutdata.h>
#include "aboutwidget.h"

LircSupport::LircSupport(const QString &name)
	: PluginBase(name, "LIRC Plugin")
{

#ifdef HAVE_LIRC_CLIENT
	kdDebug() << "initializing kradio lirc plugin\n";
	char *prg = (char*)"kradio";
	m_fd_lirc = lirc_init(prg, 1);
    m_lirc_notify = 0;
    m_lircConfig  = 0;

	if (m_fd_lirc != -1) {
		if (lirc_readconfig (NULL, &m_lircConfig, NULL) == 0) {
			m_lirc_notify = new QSocketNotifier(m_fd_lirc, QSocketNotifier::Read, this, "lirc_notifier");
			if (m_lirc_notify)
				QObject::connect(m_lirc_notify, SIGNAL(activated(int)), this, SLOT(slotLIRC(int)));
		} else {
			lirc_deinit();
			m_fd_lirc = -1;
		}
	}

	if (m_fd_lirc == -1) {
		kdDebug() << "initializing kradio lirc plugin failed\n";
	} else {
		kdDebug() << "initializing kradio lirc plugin successful\n";
	}
#endif	
	
	m_kbdTimer = new QTimer (this);
	QObject::connect (m_kbdTimer, SIGNAL(timeout()), this, SLOT(slotKbdTimedOut()));
	
	m_addIndex = 0;
}


LircSupport::~LircSupport()
{
#ifdef HAVE_LIRC_CLIENT
	if (m_fd_lirc != -1)
		lirc_deinit();
	if (m_lircConfig)
		lirc_freeconfig(m_lircConfig);
	m_fd_lirc = -1;
	m_lircConfig = 0;
#endif
}


void LircSupport::slotLIRC(int /*socket*/ )
{
#ifdef HAVE_LIRC_CLIENT
	if (!m_lircConfig || !m_lirc_notify || m_fd_lirc == -1)
		return;

	IRadioDevice *currentDevice = queryActiveDevice();
	ISeekRadio  *seeker = dynamic_cast<ISeekRadio*> (currentDevice);
	IRadioSound *sound  = dynamic_cast<IRadioSound*>(currentDevice);

	char *code = 0, *c = 0;
	if (lirc_nextcode(&code) == 0) {
		while(lirc_code2char (m_lircConfig, code, &c) == 0 && c != NULL) {
		
			if (strcasecmp (c, "TV") == 0) {
				sendPowerOff();				
			} else if (strcasecmp (c, "RADIO") == 0) {
				sendPowerOn();				
			} else if (strcasecmp (c, "POWER") == 0) {
				if (queryIsPowerOn()) sendPowerOff();
				else                  sendPowerOn();
			}
			
			if (queryIsPowerOn()) {
				if (sound && strcasecmp (c, "VOL+") == 0) {
					sound->setVolume (sound->getVolume() + 1.0/32.0);
					
				} else if (sound && strcasecmp (c, "VOL-") == 0) {
					sound->setVolume (sound->getVolume() - 1.0/32.0);
					
				} else if (   (strcasecmp (c, "CH+") == 0)
				           || (strcasecmp (c, "NEXT") == 0))
				{
					int k = queryCurrentStationIdx() + 1;
					if (k >= queryStations().count())
						k = 0;
					sendActivateStation(k);
					
				} else if (   (strcasecmp (c, "CH-") == 0)
				           || (strcasecmp (c, "PREV") == 0))
				{
					int k = queryCurrentStationIdx() - 1;
					if (k < 0)
						k = queryStations().count() - 1;
					sendActivateStation(k);
					
				} else if (seeker && strcasecmp (c, "CH+SEARCH") == 0) {
					seeker->startSeekUp();
					
				} else if (strcasecmp (c, "CH-SEARCH") == 0) {
					seeker->startSeekDown();
					
				} else if (strcasecmp (c, "SLEEP") == 0) {
					sendStartCountdown();
					
				} else if (strcasecmp (c, "QUIT") == 0) {
					kapp->quit();
				}
			
				int k = -1;
				if (sscanf (c, "%i", &k) == 1 ) {
					if (m_addIndex || k == 0) {
					    activateStation(m_addIndex * 10 + k);
					    m_kbdTimer->stop();
					    m_addIndex = 0;
					} else {
						m_addIndex = k;
						m_kbdTimer->start(500, true);
					}
				}
			}
		}
	}
	
	if (code)
		free (code);
#endif
}


void LircSupport::slotKbdTimedOut()
{
	activateStation (m_addIndex);
	m_addIndex = 0;
}


void LircSupport::activateStation (int i)
{
	if (! sendActivateStation(i - 1))
		sendActivateStation( (i + 9) % 10);
}


bool LircSupport::connect (Interface *i)
{
	bool a = IRadioClient::connect (i);
	bool b = ITimeControlClient::connect (i);
	bool c = IRadioDevicePoolClient::connect (i);
/*
    if (a) kdDebug() << "LircSupport: IRadioClient connected\n";
    if (b) kdDebug() << "LircSupport: ITimeControlClient connected\n";
    if (c) kdDebug() << "LircSupport: IRadioDevicePoolClient connected\n";
*/
	return a || b || c;
}


bool LircSupport::disconnect (Interface *i)
{
	bool a = IRadioClient::disconnect (i);
	bool b = ITimeControlClient::disconnect (i);
	bool c = IRadioDevicePoolClient::disconnect (i);
/*
    if (a) kdDebug() << "LircSupport: IRadioClient disconnected\n";
    if (b) kdDebug() << "LircSupport: ITimeControlClient disconnected\n";
    if (c) kdDebug() << "LircSupport: IRadioDevicePoolClient disconnected\n";
*/
	return a || b || c;
}



void   LircSupport::saveState (KConfig *) const
{
	// FIXME
}

void   LircSupport::restoreState (KConfig *)
{
    // FIXME
}

ConfigPageInfo LircSupport::createConfigurationPage()
{
	// FIXME
	return ConfigPageInfo ();
}

AboutPageInfo LircSupport::createAboutPage()
{
    KAboutData aboutData("kradio",
						 NULL,
                         NULL,
                         I18N_NOOP("Linux Infrared Remote Control Support for KRadio"),
                         KAboutData::License_GPL,
                         "(c) 2002, 2003 Martin Witte",
                         0,
                         "http://sourceforge.net/projects/kradio",
                         0);
    aboutData.addAuthor("Martin Witte",  "", "witte@kawo1.rwth-aachen.de");

	return AboutPageInfo(
	          new KRadioAboutWidget(aboutData, KRadioAboutWidget::AbtTabbed),
	          i18n("LIRC Support"),
	          i18n("LIRC Plugin"),
	          "connect_creating"
		   );
}
