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
#include "radiobase.h"


LircSupport::LircSupport(RadioBase *_parent)
	: QObject(_parent)
{
	radio = _parent;

#ifdef HAVE_LIRC_CLIENT
	char *prg = (char*)"kradio";
	fd_lirc = lirc_init(prg, 1);
    lirc_notify = 0;
    lircConfig  = 0;

	if (fd_lirc != -1) {
		if (lirc_readconfig (NULL, &lircConfig, NULL) == 0) {
			lirc_notify = new QSocketNotifier(fd_lirc, QSocketNotifier::Read, this, "lirc_notifier");
			if (lirc_notify)
				connect(lirc_notify, SIGNAL(activated(int)), this, SLOT(slotLIRC(int)));
		} else {
			lirc_deinit();
			fd_lirc = -1;
		}
	}
#endif	
	
	kbdTimer = new QTimer (this);
	connect (kbdTimer, SIGNAL(timeout()), this, SLOT(slotKbdTimedOut()));
	
	addIndex = 0;
}


LircSupport::~LircSupport()
{
#ifdef HAVE_LIRC_CLIENT
	if (fd_lirc != -1)
		lirc_deinit();
	if (lircConfig)
		lirc_freeconfig(lircConfig);
	fd_lirc = -1;
	lircConfig = 0;
#endif
}


void LircSupport::slotLIRC(int /*socket*/ )
{
#ifdef HAVE_LIRC_CLIENT
	if (!lircConfig || !lirc_notify || fd_lirc == -1)
		return;

	char *code = 0,
		 *c = 0;
	if (lirc_nextcode(&code) == 0) {
		while(lirc_code2char (lircConfig, code, &c) == 0 && c != NULL) {
			if (strcasecmp (c, "TV") == 0) {
				radio->PowerOff();
			} else if (strcasecmp (c, "POWER") == 0) {
				if (radio->isPowerOn())
					radio->PowerOff();
				else
					radio->PowerOn();
			}
			
			if (radio->isPowerOn()) {
				     if (strcasecmp (c, "VOL+") == 0)
					radio->setVolume (radio->getVolume() + 1.0/32.0);
				else if (strcasecmp (c, "VOL-") == 0)
					radio->setVolume (radio->getVolume() - 1.0/32.0);
				else if (strcasecmp (c, "CH+") == 0)
					radio->setFrequency(radio->getFrequency() + radio->deltaF());
				else if (strcasecmp (c, "CH-") == 0)
					radio->setFrequency(radio->getFrequency() - radio->deltaF());
				else if (strcasecmp (c, "CH+SEARCH") == 0)
					radio->startSeekUp();
				else if (strcasecmp (c, "CH-SEARCH") == 0)
					radio->startSeekDown();
			
				int k = -1;
				if (sscanf (c, "%i", &k) == 1 ) {
					if (addIndex || k == 0) {
						if (!addIndex && k == 0) k = 10;	// interpret single 0 as 10
					    activateStation(addIndex * 10 + k);
					    kbdTimer->stop();
					    addIndex = 0;
					} else {
						addIndex = k;
						kbdTimer->start(333, true);
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
	activateStation (addIndex);
	addIndex = 0;
}


void LircSupport::activateStation (int i)
{
	int im10=i%10;
	RadioStation *s = radio->getStation(i - 1);	
	if (!s) s = radio->getStation(im10 ? im10 - 1 : 9);
	if (s)
		s->activate();
}

