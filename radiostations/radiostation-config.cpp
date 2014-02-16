/***************************************************************************
                          radiostation-config.cpp  -  description
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

#include <QtCore/QTextCodec>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QLayout>
#include <klocale.h>
#include <kurlrequester.h>
#include <kcombobox.h>

#include <math.h>

#include "radiostation-config.h"
#include "frequencyradiostation.h"
#include "internetradiostation.h"


RadioStationConfig::RadioStationConfig(QWidget *parent)
    : QWidget (parent)
{

}

RadioStationConfig::~RadioStationConfig()
{
}


///////////////////////////////////////////////////////////////////////

UndefinedRadioStationConfig::UndefinedRadioStationConfig (QWidget *parent)
    : RadioStationConfig(parent)
{
    new QLabel (i18n("I do not know how to edit this station"), this);
}

UndefinedRadioStationConfig::~UndefinedRadioStationConfig()
{
}

void UndefinedRadioStationConfig::setStationData   (const RadioStation &/*rs*/)
{
}

void UndefinedRadioStationConfig::storeStationData (RadioStation &/*rs*/)
{
}


///////////////////////////////////////////////////////////////////////

FrequencyRadioStationConfig::FrequencyRadioStationConfig (QWidget *parent)
    : RadioStationConfig(parent)
{
    QHBoxLayout *hl = new QHBoxLayout(this);
    QVBoxLayout *vl = new QVBoxLayout();
    hl->setSpacing(0);
    vl->setSpacing(0);
    hl->addLayout(vl);
    vl->addWidget (new QLabel(i18n("Frequency:"), this));
    m_editFrequency = new QSpinBox(this);
    m_editFrequency->setRange(20, 150000);
    m_editFrequency->setSingleStep(10);
    vl->addWidget (m_editFrequency);
    hl->addItem(new QSpacerItem (10, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));

    connect (m_editFrequency, SIGNAL(valueChanged(int)), this, SLOT(slotValueChanged(int)));
}

FrequencyRadioStationConfig::~FrequencyRadioStationConfig()
{
}

void FrequencyRadioStationConfig::setStationData   (const RadioStation &x)
{
    const FrequencyRadioStation *rs = dynamic_cast<const FrequencyRadioStation*>(&x);
    if (rs) {
        m_editFrequency->setValue((int)rint(rs->frequency() * 1000));
    }
}

void FrequencyRadioStationConfig::storeStationData (RadioStation &x)
{
    FrequencyRadioStation *rs = dynamic_cast<FrequencyRadioStation*>(&x);
    if (rs) {
        rs->setFrequency(0.001 * m_editFrequency->value());
    }
}

void FrequencyRadioStationConfig::slotValueChanged(int /*i*/)
{
    emit changed(this);
}




///////////////////////////////////////////////////////////////////////


InternetRadioStationConfig::InternetRadioStationConfig (QWidget *parent)
    : RadioStationConfig(parent)
{
    QHBoxLayout *hl = new QHBoxLayout(this);
    QVBoxLayout *vl = new QVBoxLayout();
    hl->setSpacing(0);
    hl->setMargin (0);
    vl->setSpacing(0);
    vl->setMargin (0);

    hl->addLayout(vl);

    vl->addWidget (new QLabel(i18n("URL:"), this));
    m_editUrl = new KUrlRequester(this);
    vl->addWidget (m_editUrl);

    vl->addWidget (new QLabel(i18n("Decoder Class:"), this));
    m_comboDecoderClass = new KComboBox(this);
    vl->addWidget (m_comboDecoderClass);

    vl->addWidget (new QLabel(i18n("Playlist Class:"), this));
    m_comboPlaylistClass = new KComboBox(this);
    vl->addWidget (m_comboPlaylistClass);

    m_comboDecoderClass->clear();
    m_comboDecoderClass->addItem(i18nc("Stream Format", "<autodetect>"),    QVariant(""));
    m_comboDecoderClass->addItem(i18nc("Stream Format", "MP3"),             QVariant("mp3"));
    m_comboDecoderClass->addItem(i18nc("Stream Format", "ACC/ACCP"),        QVariant("acc"));
    m_comboDecoderClass->addItem(i18nc("Stream Format", "ASF"),             QVariant("asf"));
//     m_comboDecoderClass->addItem(i18nc("Stream Format", "ASF Stream"),      QVariant("asf_stream"));
    m_comboDecoderClass->addItem(i18nc("Stream Format", "Ogg/Vorbis"),      QVariant("ogg"));

    m_comboPlaylistClass->clear();
    m_comboPlaylistClass->addItem(i18nc("Playlist Format", "<autodetect>"), QVariant("auto"));
    m_comboPlaylistClass->addItem(i18nc("Playlist Format", "<none>"),       QVariant(""));
    m_comboPlaylistClass->addItem(i18nc("Playlist Format", "ASX"),          QVariant("asx"));
    m_comboPlaylistClass->addItem(i18nc("Playlist Format", "LSC"),          QVariant("lsc"));
    m_comboPlaylistClass->addItem(i18nc("Playlist Format", "M3U"),          QVariant("m3u"));
    m_comboPlaylistClass->addItem(i18nc("Playlist Format", "PLS"),          QVariant("pls"));
    m_comboPlaylistClass->addItem(i18nc("Playlist Format", "WMV"),          QVariant("wmv"));


    vl->addWidget (new QLabel(i18n("Meta-Data Stream Encoding:"), this));
    m_comboMetaDataEncoding = new KComboBox(this);
    vl->addWidget (m_comboMetaDataEncoding);

    initCodecList();


    connect (m_editUrl,               SIGNAL(textChanged(const QString& )), this, SLOT(slotUrlChanged(const QString &)));
    connect (m_comboDecoderClass,     SIGNAL(currentIndexChanged(int)),     this, SLOT(slotDecoderClassChanged(int)));
    connect (m_comboPlaylistClass,    SIGNAL(currentIndexChanged(int)),     this, SLOT(slotPlaylistClassChanged(int)));
    connect (m_comboMetaDataEncoding, SIGNAL(currentIndexChanged(int)),     this, SLOT(slotMetadataEncodingChanged(int)));
}

InternetRadioStationConfig::~InternetRadioStationConfig()
{
}


void InternetRadioStationConfig::setStationData   (const RadioStation &x)
{
    const InternetRadioStation *rs = dynamic_cast<const InternetRadioStation*>(&x);
    if (rs) {
        m_editUrl->setUrl(rs->url());
        int idx = m_comboDecoderClass->findData(QVariant(rs->decoderClass()));
        if(idx >= 0) {
            m_comboDecoderClass->setCurrentIndex(idx);
        }
        idx = m_comboPlaylistClass->findData(QVariant(rs->playlistClass()));
        if(idx >= 0) {
            m_comboPlaylistClass->setCurrentIndex(idx);
        }
        idx = m_comboMetaDataEncoding->findData(QVariant(rs->metaDataEncoding()));
        if(idx >= 0) {
            m_comboMetaDataEncoding->setCurrentIndex(idx);
        }
    }
}


void InternetRadioStationConfig::storeStationData (RadioStation &x)
{
    InternetRadioStation *rs = dynamic_cast<InternetRadioStation*>(&x);
    if (rs) {
        rs->setUrl(m_editUrl->url());
        rs->setDecoderClass    (m_comboDecoderClass    ->itemData(m_comboDecoderClass    ->currentIndex()).value<QString>());
        rs->setPlaylistClass   (m_comboPlaylistClass   ->itemData(m_comboPlaylistClass   ->currentIndex()).value<QString>());
        rs->setMetaDataEncoding(m_comboMetaDataEncoding->itemData(m_comboMetaDataEncoding->currentIndex()).value<QString>());
    }
}

void InternetRadioStationConfig::slotUrlChanged(const QString &)
{
    emit changed(this);
}

void InternetRadioStationConfig::slotDecoderClassChanged(int /*idx*/)
{
    emit changed(this);
}

void InternetRadioStationConfig::slotPlaylistClassChanged(int /*idx*/)
{
    emit changed(this);
}


void InternetRadioStationConfig::slotMetadataEncodingChanged(int /*idx*/)
{
    emit changed(this);
}


void InternetRadioStationConfig::initCodecList() const
{
    QMap<QString, QString> codecs;

    foreach (const QByteArray &_name, QTextCodec::availableCodecs()) {
        QString name = _name;
        codecs.insert(name.toUpper(), name);
    }
    QStringList sortedKeys = codecs.keys();
    sortedKeys.sort();
    m_comboMetaDataEncoding->addItem(i18n("auto"), QVariant("auto"));
    foreach (QString key, sortedKeys) {
        m_comboMetaDataEncoding->addItem(codecs[key], QVariant(key));
    }
}


#include "radiostation-config.moc"
