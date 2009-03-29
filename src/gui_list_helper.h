/***************************************************************************
                              gui_list_helper.h
                             -------------------
    begin                : Son Sep 26 2004
    copyright            : (C) 2004 by Martin Witte
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

#ifndef _KRADIO_LIBKRADIO_GUI_GUI_LIST_HELPER_H_
#define _KRADIO_LIBKRADIO_GUI_GUI_LIST_HELPER_H_

#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QString>

template <class TLIST> class KDE_EXPORT GUISimpleListHelper
{
public:
    GUISimpleListHelper(TLIST *list) : m_List(list) {}
    ~GUISimpleListHelper() {}

    void setList(TLIST *list);

    void     setData(const QList<QString> &data);
    QString  getCurrentText() const           { return m_List->currentText(); }
    void     setCurrentText(const QString &s) { m_List->setCurrentIndex(m_revData.contains(s) ? m_revData[s] : 0); }

    int count() const { return m_revData.count(); }
    bool contains(const QString &id) const { return m_revData.contains(id); }

protected:
    TLIST              *m_List;
    QMap<QString, int>  m_revData;
};

template <class TLIST>
void GUISimpleListHelper<TLIST>::setList(TLIST *list)
{
    m_List = list;
}

template <class TLIST>
void GUISimpleListHelper<TLIST>::setData(const QList<QString> &data)
{
    m_List->clear();
    m_revData.clear();

    QList<QString>::const_iterator it  = data.begin();
    QList<QString>::const_iterator end = data.end();
    for (int i = 0; it != end; ++it, ++i) {
        m_revData[*it] = i;
        m_List->insertItem(i, *it);
    }
}









template <class TLIST, class TID> class GUIListHelper
{
public:
    enum SORT_KEY { SORT_BY_ID, SORT_BY_DESCR };

    GUIListHelper(TLIST *list, SORT_KEY skey);
    GUIListHelper(TLIST *list, const QMap<TID, QString> &data, SORT_KEY skey);
    ~GUIListHelper();

    void setList(TLIST *list);

    void setData(const QMap<TID, QString> &data);

    void       setCurrentItem(const TID &) const;
    const TID  getCurrentItem()    const;

    int count() const { return m_Index2ID.count(); }

    bool contains(const TID &id) const { return m_ID2Index.contains(id); }

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
        bool operator > (const THelpData &d) const { return (skey == SORT_BY_ID) ? id > d.id : descr > d.descr; }
        bool operator < (const THelpData &d) const { return (skey == SORT_BY_ID) ? id < d.id : descr < d.descr; }
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
void GUIListHelper<TLIST, TID>::setList(TLIST *list)
{
    m_List = list;
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
    QList<THelpData>          help_list;
    QMapIterator<TID, QString> it(data);
    while(it.hasNext()) {
        it.next();
        help_list.push_back(THelpData(it.key(), it.value(), m_skey));
    }
    qSort(help_list);

    m_Index2ID.clear();
    m_ID2Index.clear();

    int idx = 0;
    QListIterator<THelpData> it2(help_list);
    while(it2.hasNext()) {
        const THelpData &hd = it2.next();
        m_Index2ID.insert(idx, hd.id);
        m_ID2Index.insert(hd.id, idx);
        m_List->insertItem(idx, hd.descr);
        ++idx;
    }
}


template <class TLIST, class TID>
void GUIListHelper<TLIST, TID>::setCurrentItem(const TID &id) const
{
    if (m_ID2Index.contains(id))
        m_List->setCurrentIndex(m_ID2Index[id]);
    else
        m_List->setCurrentIndex(0);
}

template <class TLIST, class TID>
const TID GUIListHelper<TLIST, TID>::getCurrentItem() const
{
    int idx = m_List->currentIndex();
    return (idx >= 0) ? (m_Index2ID.begin() + idx).value() : TID();
}

#endif
