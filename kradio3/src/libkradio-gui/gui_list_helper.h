/***************************************************************************
                              gui_list_helper.h
                             -------------------
    begin                : Son Sep 26 2004
    copyright            : (C) 2004 by Martin Witte
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

#ifndef _KRADIO_LIBKRADIO_GUI_GUI_LIST_HELPER_H_
#define _KRADIO_LIBKRADIO_GUI_GUI_LIST_HELPER_H_

#include <qmap.h>
#include <qvaluelist.h>


template <class TLIST, class TID> class GUIListHelper
{
public:
    enum SORT_KEY { SORT_BY_ID, SORT_BY_DESCR };

    GUIListHelper(TLIST *list, SORT_KEY skey);
    GUIListHelper(TLIST *list, const QMap<TID, QString> &data, SORT_KEY skey);
    ~GUIListHelper();

    void setData(const QMap<TID, QString> &data);

    void setCurrentItem(TID) const;
    TID  getCurrentItem()    const;

    int count() const { return m_Index2ID.count(); }

protected:
    SORT_KEY           m_skey;
    TLIST             *m_List;
    QMap<int, TID>     m_Index2ID;
    QMap<TID, int>     m_ID2Index;
    QMap<TID, QString> m_ID2Description;

    struct THelpData {
        TID      id;
        QString  descr;
        SORT_KEY skey;

        THelpData() : id(), descr(), skey(SORT_BY_ID) {}
        THelpData(TID _id, const QString &_descr, SORT_KEY _skey)
            : id(_id),
              descr(_descr),
              skey(_skey)
          {}
        bool operator > (const THelpData &d) { return (skey == SORT_BY_ID) ? id > d.id : descr > d.descr; }
        bool operator < (const THelpData &d) { return (skey == SORT_BY_ID) ? id < d.id : descr < d.descr; }
    };
};


template <class TLIST, class TID>
GUIListHelper<TLIST, TID>::GUIListHelper(TLIST *list, SORT_KEY skey)
    : m_skey(skey),
      m_List(list)
{
}


template <class TLIST, class TID>
GUIListHelper<TLIST, TID>::GUIListHelper(TLIST *list, const QMap<TID, QString> &data, SORT_KEY skey)
    : m_skey(skey),
      m_List(list)
{
    setData(data);
}


template <class TLIST, class TID>
GUIListHelper<TLIST, TID>::~GUIListHelper()
{
}


template <class TLIST, class TID>
void GUIListHelper<TLIST, TID>::setData (const QMap<TID, QString> &data)
{
    m_List->clear();

    m_ID2Description = data;
    QValueList<THelpData> help_list;
    for (QMapConstIterator<TID, QString> it = data.begin(); it != data.end(); ++it) {
        help_list.push_back(THelpData(it.key(), *it, m_skey));
    }
    qHeapSort(help_list);

    m_Index2ID.clear();
    m_ID2Index.clear();

    int idx = 0;
    for (QValueListIterator<THelpData> it = help_list.begin(); it != help_list.end(); ++it, ++idx) {
        m_Index2ID.insert(idx, (*it).id);
        m_ID2Index.insert((*it).id, idx);
        m_List->insertItem((*it).descr);
    }
}


template <class TLIST, class TID>
void GUIListHelper<TLIST, TID>::setCurrentItem(TID id) const
{
    if (m_ID2Index.contains(id))
        m_List->setCurrentItem(m_ID2Index[id]);
    else
        m_List->setCurrentItem(0);
}

template <class TLIST, class TID>
TID GUIListHelper<TLIST, TID>::getCurrentItem() const
{
    int idx = m_List->currentItem();
    return m_Index2ID[idx];
}

#endif