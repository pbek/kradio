/***************************************************************************
                          stationselector.cpp  -  description
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

#include "stationselector.h"
#include "stationlist.h"
#include "radiostation.h"

#include <klistbox.h>
#include <kpushbutton.h>

#include <algorithm>

using namespace std;

StationSelector::StationSelector (QWidget *parent)
	: StationSelectorUI(parent)
{
	QObject::connect(buttonToLeft,  SIGNAL(clicked()), this, SLOT(slotButtonToLeft()));
	QObject::connect(buttonToRight, SIGNAL(clicked()), this, SLOT(slotButtonToRight()));
}


StationSelector::~StationSelector ()
{
}


bool StationSelector::connect(Interface *i)
{
	bool a = IStationSelectionClient::connect(i);
	bool b = IRadioClient::connect(i);
	return a || b;
}


bool StationSelector::disconnect(Interface *i)
{
	bool a = IStationSelectionClient::disconnect(i);
	bool b = IRadioClient::disconnect(i);
	return a || b;
}


bool StationSelector::noticeStationSelectionChanged(const QStringList &sl)
{
	stationsNotDisplayed.clear();

    while (listSelected->count()) {
		moveItem (listSelected, stationsSelected, 0, listAvailable, stationsAvailable);
    }

	for (QStringList::const_iterator it = sl.begin(); it != sl.end(); ++it) {
		const QString &id = *it;

        if (::find(stationsAvailable.begin(), stationsAvailable.end(), id) != stationsAvailable.end()) {
			// get index in stationsAvailable
			unsigned int idx = 0;
			while (idx < stationsAvailable.size() && stationsAvailable[idx] != id) ++idx;
			if (idx < stationsAvailable.size())
				moveItem(listAvailable, stationsAvailable, idx, listSelected, stationsSelected);
        } else {
			stationsNotDisplayed.push_back(id);
        }
	}
	return true;
}


bool StationSelector::noticePowerChanged(bool /*on*/)
{
	return false;
}


bool StationSelector::noticeStationChanged (const RadioStation &, int /*idx*/)
{
	return false;
}


bool StationSelector::noticeStationsChanged(const StationList &sl)
{
	listAvailable->clear();
	stationsAvailable.clear();
	listSelected->clear();
	stationsNotDisplayed.insert(stationsNotDisplayed.begin(), stationsSelected.begin(), stationsSelected.end());
	stationsSelected.clear();

	for (RawStationList::Iterator i(sl.all()); i.current(); ++i) {
	    const QString &id = i.current()->stationID();

	    stationsAll.push_back(id);

	    vector<QString>::iterator f = ::find(stationsNotDisplayed.begin(), stationsNotDisplayed.end(), id);
	    if (f != stationsNotDisplayed.end()) {
			listSelected->insertItem(i.current()->iconName(),
			                         i.current()->longName());
			stationsSelected.push_back(id);
			stationsNotDisplayed.erase(f);
	    } else {
			listAvailable->insertItem(i.current()->iconName(),
			                          i.current()->longName());
			stationsAvailable.push_back(id);
		}
	}
	return true;
}


void StationSelector::slotButtonToLeft()
{
	unsigned int i = 0;
	while (i < listSelected->count()) {
		if (listSelected->isSelected(i)) {
			moveItem(listSelected, stationsSelected,
				     i,
				     listAvailable, stationsAvailable);
		} else {
			++i;
		}
	}
}


void StationSelector::slotButtonToRight()
{
	unsigned int i = 0;
	while (i < listAvailable->count()) {
		if (listAvailable->isSelected(i)) {
			moveItem(listAvailable, stationsAvailable,
				     i,
				     listSelected, stationsSelected);
		} else {
			++i;
		}
	}
}

void StationSelector::moveItem (
	KListBox *&lbFrom, vector<QString> &vFrom,
    unsigned int idxFrom,
	KListBox *&lbTo, vector<QString> &vTo)
{
	QString         id  =  vFrom[idxFrom];
	QString         txt =  lbFrom->text(idxFrom);
	const QPixmap  *p   =  lbFrom->pixmap(idxFrom);

	insertItem (lbTo, vTo, id, p, txt);

	vFrom.erase(::find(vFrom.begin(), vFrom.end(), id));
	lbFrom->removeItem(idxFrom);
}


void StationSelector::insertItem (
	KListBox *&lb,
	vector<QString> &v,
	const QString &id,
	const QPixmap *p,
	const QString &txt)
{
	// calculate where to insert the new item
	unsigned int k = 0,
                idx = 0;
	for (; idx < v.size(); ++idx) {

		while (   k  <  stationsAll.size()
		       && id != stationsAll[k]
		       && v[idx] != stationsAll[k])
		{ ++k; }

		if (k < stationsAll.size() && id == stationsAll[k])
			break;
	}

	vector<QString>::iterator  it = v.begin();
	unsigned int c = 0;
	while (it != v.end() && c < idx) ++it, ++c;

	v.insert(it, id);
	if (p)
		lb->insertItem(*p, txt, idx);
	else
		lb->insertItem(txt, idx);
}


void StationSelector::slotOK()
{
	QStringList l;
	for (vector<QString>::iterator it = stationsSelected.begin(); it != stationsSelected.end(); ++it)
		l.push_back(*it);
	for (vector<QString>::iterator it = stationsNotDisplayed.begin(); it != stationsNotDisplayed.end(); ++it)
		l.push_back(*it);
	sendStationSelection(l);
}


void StationSelector::slotCancel()
{
    noticeStationSelectionChanged(queryStationSelection());
}


