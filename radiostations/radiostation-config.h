/***************************************************************************
                          radiostation-config.h  -  description
                             -------------------
    begin                : Sa Aug 16 2003
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

#ifndef KRADIO_RADIOSTATION_CONFIG_H
#define KRADIO_RADIOSTATION_CONFIG_H

#include <QWidget>
#include "kradio-def.h"

class RadioStation;

class KRADIO5_EXPORT RadioStationConfig : public QWidget
{
Q_OBJECT
public:
	RadioStationConfig(QWidget *parent);
    virtual ~RadioStationConfig();

    virtual void setStationData   (const RadioStation &rs) = 0;
    virtual void storeStationData (RadioStation &rs) = 0;

signals:
            void changed(RadioStationConfig *);
};


class KRADIO5_EXPORT UndefinedRadioStationConfig : public RadioStationConfig
{
Q_OBJECT
public:
	UndefinedRadioStationConfig (QWidget *parent);
	~UndefinedRadioStationConfig();

    virtual void setStationData   (const RadioStation &rs) override;
    virtual void storeStationData (RadioStation &rs)       override;
};



class QSpinBox;

class KRADIO5_EXPORT FrequencyRadioStationConfig : public RadioStationConfig
{
Q_OBJECT
public:
	FrequencyRadioStationConfig (QWidget *parent);
	~FrequencyRadioStationConfig();

    virtual void setStationData   (const RadioStation &rs) override;
    virtual void storeStationData (RadioStation &rs)       override;

protected slots:
	virtual void slotValueChanged(int);

protected:

	QSpinBox *m_editFrequency;
};


class KUrlRequester;
class KComboBox;
class KRADIO5_EXPORT InternetRadioStationConfig : public RadioStationConfig
{
Q_OBJECT
public:
    InternetRadioStationConfig (QWidget *parent);
    ~InternetRadioStationConfig();

    virtual void setStationData   (const RadioStation &rs) override;
    virtual void storeStationData (RadioStation &rs)       override;
    
protected:
    void         initCodecList() const;

protected slots:
    virtual void slotUrlChanged(const QString &);
    virtual void slotDecoderClassChanged(int idx);
    virtual void slotPlaylistClassChanged(int idx);
    virtual void slotMetadataEncodingChanged(int idx);

protected:

    KUrlRequester *m_editUrl;
    KComboBox     *m_comboDecoderClass;
    KComboBox     *m_comboPlaylistClass;
    KComboBox     *m_comboMetaDataEncoding;
};




#endif
