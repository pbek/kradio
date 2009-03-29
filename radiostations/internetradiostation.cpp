/***************************************************************************
                          internetradiostation.cpp  -  description
                             -------------------
    begin                : Sat March 29 2003
    copyright            : (C) 2003 by Klas Kalass
    email                : klas@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "internetradiostation.h"
#include <typeinfo>
#include "radiostation-config.h"

/////////////////////////////////////////////////////////////////////////////

const char *StationUrlElement           = "url";
const char *StationDecoderClassElement  = "decoderclass";
const char *StationPlaylistClassElement = "playlistclass";

static InternetRadioStation  emptyInternetRadioStation(registerStationClass);

/////////////////////////////////////////////////////////////////////////////

InternetRadioStation::InternetRadioStation()
    : RadioStation(),
      m_url(),
      m_decoderClass(),
      m_playlistClass("auto")
{
}

InternetRadioStation::InternetRadioStation(const KUrl &url, const QString &decoder_class, const QString &playlist_class)
    : RadioStation(),
      m_url(url),
      m_decoderClass(decoder_class),
      m_playlistClass(playlist_class)
{
}

InternetRadioStation::InternetRadioStation(const QString &name,
                                           const QString &shortName,
                                           const KUrl    &url,
                                           const QString &decoder_class,
                                           const QString &playlist_class)
    : RadioStation(name, shortName),
      m_url(url),
      m_decoderClass(decoder_class),
      m_playlistClass(playlist_class)
{
}

InternetRadioStation::InternetRadioStation(const InternetRadioStation &s)
    : RadioStation(s),
      m_url(s.m_url),
      m_decoderClass (s.m_decoderClass),
      m_playlistClass(s.m_playlistClass)
{
}


InternetRadioStation::InternetRadioStation(RegisterStationClass, const QString &classname)
    : RadioStation(registerStationClass, !classname.isNull() ? classname : getClassName()),
      m_url(),
      m_decoderClass(),
      m_playlistClass("auto")
{
}


/** returns an exact copy of this station*/
RadioStation *InternetRadioStation::copy() const
{
    return new InternetRadioStation(*this);
}

RadioStation *InternetRadioStation::copyNewID() const
{
    RadioStation *x = new InternetRadioStation(*this);
    x->generateNewStationID();
    return x;
}

InternetRadioStation::~InternetRadioStation()
{
}


/*  = 0 : this.url = s.url
    > 0 : this.url > s.url
    < 0 : this.url < s.url
    other class than FrequencyRadioStation: compare typeid(.).name()
*/
int InternetRadioStation::compare(const RadioStation &_s) const
{
    InternetRadioStation const *s = dynamic_cast<InternetRadioStation const*>(&_s);

    if (!s)
        return (typeid(this).name() > typeid(&_s).name()) ? 1 : -1;

    QString thisurl = m_url.url(KUrl::RemoveTrailingSlash);    // -1: remove trailing '/'
    QString thaturl = s->m_url.url(KUrl::RemoveTrailingSlash);

    // empty urls are never identical
    if (thisurl.length () == 0)
        return -1;
    if (thaturl.length() == 0)
        return 1;

    int url_cmp = thisurl.compare(thaturl);
    int dec_cmp = m_decoderClass.compare (s->m_decoderClass);
    int pls_cmp = m_playlistClass.compare(s->m_playlistClass);
    if (url_cmp == 0) {
        if (pls_cmp == 0) {
            return dec_cmp;
        } else {
            return pls_cmp;
        }
    } else {
        return url_cmp;
    }
}



bool InternetRadioStation::isValid() const
{
    // TODO: maybe we need to do more to validate this...
    return !m_url.isEmpty();
}

QString InternetRadioStation::longName() const
{
    QString longN = name();
    if (!longN.isEmpty()) {
        longN = i18n("%1, %2", longN, description());
    } else {
        longN = description();
    }

    return longN;
}


QString InternetRadioStation::description() const
{
    return m_url.pathOrUrl();
}


bool InternetRadioStation::setProperty(const QString &pn, const QString &val)
{
    bool retval = false;
    if (pn == StationUrlElement) {
        m_url  = val;
        retval = true;
    } else if (pn == StationDecoderClassElement) {
        m_decoderClass = val;
        retval         = true;
    } else if (pn == StationPlaylistClassElement) {
        m_playlistClass = val;
        retval          = true;
    } else {
        retval = RadioStation::setProperty(pn, val);
    }
    return retval;
}

QString InternetRadioStation::getProperty(const QString &pn) const
{
    if (pn == StationUrlElement) {
        return m_url.pathOrUrl();
    } else if (pn == StationDecoderClassElement) {
        return m_decoderClass;
    } else if (pn == StationPlaylistClassElement) {
        return m_playlistClass;
    } else {
        return RadioStation::getProperty(pn);
    }
}

QStringList InternetRadioStation::getPropertyNames() const
{
    QStringList l = RadioStation::getPropertyNames();
    l.push_back(StationUrlElement);
    l.push_back(StationDecoderClassElement);
    l.push_back(StationPlaylistClassElement);
    return l;
}


RadioStationConfig *InternetRadioStation::createEditor() const
{
    return new InternetRadioStationConfig(NULL);
}

bool InternetRadioStation::operator == (const RadioStation &x) const
{
    if (!RadioStation::operator == (x))
        return false;

    InternetRadioStation const *fx = dynamic_cast<InternetRadioStation const*>(&x);
    if (!fx)
        return false;
    return compare(*fx) == 0;
}
