/***************************************************************************
                          radiostation-config.h  -  description
                             -------------------
    begin                : Sa Aug 16 2003
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

#ifndef KRADIO_RADIOSTATION_CONFIG_H
#define KRADIO_RADIOSTATION_CONFIG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qwidget.h>

class RadioStation;

class RadioStationConfig : public QWidget
{
Q_OBJECT
public:
	RadioStationConfig(QWidget *parent);
    ~RadioStationConfig();

    virtual void setStationData   (const RadioStation &rs) = 0;
    virtual void storeStationData (RadioStation &rs) = 0;

signals:
	virtual void changed(RadioStationConfig *) = 0;
};


class UndefinedRadioStationConfig : public RadioStationConfig
{
Q_OBJECT
public:
	UndefinedRadioStationConfig (QWidget *parent);
	~UndefinedRadioStationConfig();

    virtual void setStationData   (const RadioStation &rs);
    virtual void storeStationData (RadioStation &rs);

signals:
	virtual void changed(RadioStationConfig *);
};



class QSpinBox;

class FrequencyRadioStationConfig : public RadioStationConfig
{
Q_OBJECT
public:
	FrequencyRadioStationConfig (QWidget *parent);
	~FrequencyRadioStationConfig();

    virtual void setStationData   (const RadioStation &rs);
    virtual void storeStationData (RadioStation &rs);

signals:
	virtual void changed(RadioStationConfig *);

protected slots:
	virtual void slotValueChanged(int);

protected:

	QSpinBox *m_editFrequency;
};



#endif
