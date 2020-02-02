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
#include <QtCore/QObject>
#include <QtCore/QVariant>

#include "kradio-def.h"

class KRADIO5_EXPORT GUIListHelperQObjectBase : public QObject
{
Q_OBJECT

public:
    GUIListHelperQObjectBase         ();
    virtual ~GUIListHelperQObjectBase();

public slots:

    virtual void    slotOK()            = 0;
    virtual void    slotCancel()        = 0;

protected slots:
    virtual void    slotUserSelection() = 0;

protected:
    void            emitSigDirtyChanged(bool d);

signals:
    void            sigDirtyChanged (bool dirty);
};


template <class TLIST, class TID> class KRADIO5_EXPORT GUIListHelper : public GUIListHelperQObjectBase
{
public:
    enum SORT_KEY { SORT_BY_ID, SORT_BY_DESCR, SORT_NONE };

    GUIListHelper (TLIST *list, SORT_KEY skey);
    GUIListHelper (TLIST *list, const QMap<TID, QString> &data, SORT_KEY skey);
    GUIListHelper (TLIST *list, const QList<QString> &data, SORT_KEY skey);
    ~GUIListHelper() {}

    void          setList(TLIST *list)                { m_List = list; }

    void          setCurrentItemID(const TID &id);
    void          setOrgItemID    (const TID &id);

    template<class TData>
    void          alternativesChanged(const TData &data);

    const TID     getCurrentItemID() const;

    int           count() const                       { return m_List ? m_List->count() : 0; }

public:

    virtual void  slotOK()            override;
    virtual void  slotCancel()        override;
    virtual void  slotUserSelection() override;

protected:
    void          setData(const QMap<TID, QString> &data);  // only updates list elements, no setting/update of current element
    void          setData(const QList<QString>     &data);  // only updates list elements, no setting/update of current element
    bool          containsItemID(const TID &id) const { return m_List && m_List->findData(id) >= 0; }

    void          setUserDirty()                  { setDirty(true,  m_alternativeDirty); }
    void          setUserDirty(bool dirty)        { setDirty(dirty, m_alternativeDirty); }
    void          setAlternativeDirty()           { setDirty(m_userDirty, true);  }
    void          setAlternativeDirty(bool dirty) { setDirty(m_userDirty, dirty); }
    void          setDirty(bool userDirty, bool alternativesDirty)
                                                  { m_userDirty        = userDirty;
                                                    m_alternativeDirty = alternativesDirty;
                                                    emitSigDirtyChanged(m_userDirty || m_alternativeDirty); }

protected:
    SORT_KEY      m_skey;
    TLIST        *m_List;
    bool          m_userDirty;
    bool          m_alternativeDirty;

    TID           m_orgID;
    TID           m_userSelID;

    bool          m_ignoreGUIChange;

    struct THelpData {
        TID         id;
        QString     descr;
        SORT_KEY    skey;

        THelpData()
            : id(),
              descr(),
              skey(SORT_BY_ID)
          {}

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
    m_List(list),
    m_userDirty(false),
    m_alternativeDirty(false),
    m_ignoreGUIChange(false)
{
    if (list) {
        QObject::connect(list, QOverload<int>::of(&TLIST::activated), this, &GUIListHelper::slotUserSelection);
    }
}


template <class TLIST, class TID>
GUIListHelper<TLIST, TID>::GUIListHelper(TLIST *list, const QMap<TID, QString> &data, SORT_KEY skey)
  : m_skey(skey),
    m_List(list),
    m_userDirty(false),
    m_alternativeDirty(false),
    m_ignoreGUIChange(false)
{
    if (list) {
        QObject::connect(list, QOverload<int>::of(&TLIST::activated), this, &GUIListHelper::slotUserSelection);
    }
    setData(data);
}

template <class TLIST, class TID>
GUIListHelper<TLIST, TID>::GUIListHelper(TLIST *list, const QList<QString> &data, SORT_KEY skey)
  : m_skey(skey),
    m_List(list),
    m_userDirty(false),
    m_alternativeDirty(false),
    m_ignoreGUIChange(false)
{
    if (list) {
        QObject::connect(list, QOverload<int>::of(&TLIST::activated), this, &GUIListHelper::slotUserSelection);
    }
    setData(data);
}




template <class TLIST, class TID>
void GUIListHelper<TLIST, TID>::slotOK()
{
    if (m_userDirty) {
        setOrgItemID(getCurrentItemID());
    }
    setDirty(false, !m_userDirty && m_alternativeDirty);
}

template <class TLIST, class TID>
void GUIListHelper<TLIST, TID>::slotCancel()
{
    setDirty(false, false);
    setCurrentItemID(m_orgID);  // will set alternativeDirty if org is not available
}

template <class TLIST, class TID>
void GUIListHelper<TLIST, TID>::slotUserSelection()
{
    if (m_ignoreGUIChange)
        return;
    m_userSelID = getCurrentItemID();
    setDirty(true, false);
}

template <class TLIST, class TID>
template <class TData>
void GUIListHelper<TLIST, TID>::alternativesChanged(const TData &data)
{
    setData(data);

    // m_userDirty is not touched
    setAlternativeDirty(false); // will be set if no alternative is available

    if (!m_userDirty) {
        // try to set original, alternativeDirty will be set if not possible
        setCurrentItemID(m_orgID);
    } else {
        // try to keep user selection. alternativeDirty will be set if not possible
        setCurrentItemID(m_userSelID);
    }
}



template <class TLIST, class TID>
void GUIListHelper<TLIST, TID>::setData(const QMap<TID, QString> &data) {
    m_List->clear();

    QList<THelpData> help_list;
    for (typename QMap<TID, QString>::const_iterator it = data.begin(); it != data.end(); ++it) {
        help_list.push_back(THelpData(it.key(), it.value(), m_skey));
    }
    if (m_skey != SORT_NONE) {
        qSort(help_list);
    }

    THelpData  item;
    foreach (item, help_list) {
        m_List->addItem(item.descr, item.id);
    }
}


template <class TLIST, class TID>
void GUIListHelper<TLIST, TID>::setData (const QList<QString> &_data)
{
    m_List->clear();
    QList<QString> data = _data;
    if (m_skey != SORT_NONE) {
        qSort(data);
    }

    QString item;
    foreach (item, data) {
        m_List->addItem(item, item);
    }
}


template <class TLIST, class TID>
void GUIListHelper<TLIST, TID>::setCurrentItemID(const TID &id)
{
    bool oldIgnoreGUIChange = m_ignoreGUIChange;
    m_ignoreGUIChange = true;

    int idx = m_List->findData(id);
    if (idx >= 0) {
        m_List->setCurrentIndex(idx);
    } else {
        m_List->setCurrentIndex(0);
        setAlternativeDirty();
    }

    m_ignoreGUIChange = oldIgnoreGUIChange;
}

template <class TLIST, class TID>
void GUIListHelper<TLIST, TID>::setOrgItemID(const TID &id)
{
    m_orgID = id;
    if (!m_userDirty) {
        setCurrentItemID(m_orgID);
    }
}

template <class TLIST, class TID>
const TID GUIListHelper<TLIST, TID>::getCurrentItemID() const
{
    int idx = m_List->currentIndex();
    if (idx >= 0) {
        const QVariant &data = m_List->itemData(idx);
        return data.value<TID>();
    } else {
        return TID();
    }
}



#endif
