/***************************************************************************
                       lirc-configuration.cpp  -  description
                             -------------------
    begin                : Sat May 21 2005
    copyright            : (C) 2005 by Martin Witte
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

#include <klistview.h>
#include <klocale.h>

#include <qlistview.h>
#include <qlabel.h>

#include "lirc-configuration.h"
#include "lircsupport.h"
#include "listviewitem_lirc.h"

LIRCConfiguration::LIRCConfiguration (QWidget *parent, LircSupport *dev)
 : LIRCConfigurationUI(parent),
   m_LIRC (dev),
   m_dirty(true),
   m_ignore_gui_updates(false)
{
    m_descriptions[LIRC_DIGIT_0] = i18n("digit 0");
    m_descriptions[LIRC_DIGIT_1] = i18n("digit 1");
    m_descriptions[LIRC_DIGIT_2] = i18n("digit 2");
    m_descriptions[LIRC_DIGIT_3] = i18n("digit 3");
    m_descriptions[LIRC_DIGIT_4] = i18n("digit 4");
    m_descriptions[LIRC_DIGIT_5] = i18n("digit 5");
    m_descriptions[LIRC_DIGIT_6] = i18n("digit 6");
    m_descriptions[LIRC_DIGIT_7] = i18n("digit 7");
    m_descriptions[LIRC_DIGIT_8] = i18n("digit 8");
    m_descriptions[LIRC_DIGIT_9] = i18n("digit 9");
    m_descriptions[LIRC_POWER_ON]     = i18n("Power On");
    m_descriptions[LIRC_POWER_OFF]    = i18n("Power Off");
    m_descriptions[LIRC_PAUSE]        = i18n("Pause");
    m_descriptions[LIRC_RECORD_START] = i18n("Start Recording");
    m_descriptions[LIRC_RECORD_STOP]  = i18n("Stop Recording");
    m_descriptions[LIRC_VOLUME_INC]   = i18n("Increase Volume");
    m_descriptions[LIRC_VOLUME_DEC]   = i18n("Decrease Volume");
    m_descriptions[LIRC_CHANNEL_NEXT] = i18n("Next Channel");
    m_descriptions[LIRC_CHANNEL_PREV] = i18n("Previous Channel");
    m_descriptions[LIRC_SEARCH_NEXT]  = i18n("Search Next Channel");
    m_descriptions[LIRC_SEARCH_PREV]  = i18n("Search Previous Channel");
    m_descriptions[LIRC_SLEEP]        = i18n("Enable Sleep Countdown");
    m_descriptions[LIRC_APPLICATION_QUIT] = i18n("Quit KRadio");

    int k = 0;
    m_order[k++] = LIRC_DIGIT_0;
    m_order[k++] = LIRC_DIGIT_1;
    m_order[k++] = LIRC_DIGIT_2;
    m_order[k++] = LIRC_DIGIT_3;
    m_order[k++] = LIRC_DIGIT_4;
    m_order[k++] = LIRC_DIGIT_5;
    m_order[k++] = LIRC_DIGIT_6;
    m_order[k++] = LIRC_DIGIT_7;
    m_order[k++] = LIRC_DIGIT_8;
    m_order[k++] = LIRC_DIGIT_9;
    m_order[k++] = LIRC_POWER_ON;
    m_order[k++] = LIRC_POWER_OFF;
    m_order[k++] = LIRC_PAUSE;
    m_order[k++] = LIRC_RECORD_START;
    m_order[k++] = LIRC_RECORD_STOP;
    m_order[k++] = LIRC_VOLUME_INC;
    m_order[k++] = LIRC_VOLUME_DEC;
    m_order[k++] = LIRC_CHANNEL_NEXT;
    m_order[k++] = LIRC_CHANNEL_PREV;
    m_order[k++] = LIRC_SEARCH_NEXT;
    m_order[k++] = LIRC_SEARCH_PREV;
    m_order[k++] = LIRC_SLEEP;
    m_order[k++] = LIRC_APPLICATION_QUIT;

    m_ActionList->setSorting(-1);
    m_ActionList->setColumnWidthMode(0, QListView::Maximum);
    m_ActionList->setColumnWidthMode(1, QListView::Maximum);
    m_ActionList->setColumnWidthMode(2, QListView::Maximum);

    connect(m_ActionList, SIGNAL(itemRenamed(QListViewItem*, int)), this, SLOT(slotSetDirty()));
    slotCancel();
}


LIRCConfiguration::~LIRCConfiguration ()
{
}


void LIRCConfiguration::slotOK()
{
    if (m_dirty && m_LIRC) {
        QListViewItem *item = m_ActionList->firstChild();

        QMap<LIRC_Actions, QString> actions;
        QMap<LIRC_Actions, QString> alt_actions;

        for (int i = 0; item; ++i, item = item->nextSibling()) {
            LIRC_Actions action = m_order[i];
            actions[action]     = item->text(1);
            alt_actions[action] = item->text(2);
        }
        m_LIRC->setActions(actions, alt_actions);
    }
    m_dirty = false;
}


void LIRCConfiguration::slotCancel()
{
    if (m_dirty) {
        m_ignore_gui_updates = true;
        m_ActionList->clear();
        if (m_LIRC) {
            const QMap<LIRC_Actions, QString> &actions     = m_LIRC->getActions();
            const QMap<LIRC_Actions, QString> &alt_actions = m_LIRC->getAlternativeActions();

            for (unsigned i = 0; m_order.contains(i) && i < m_order.count(); ++i) {
                LIRC_Actions action = m_order[i];
                addKey(m_descriptions[action], actions[action], alt_actions[action]);
            }
        }

        slotRenamingStopped(NULL, -1);
        m_ignore_gui_updates = false;
    }
    m_dirty = false;
}


void LIRCConfiguration::addKey(const QString &descr, const QString &key, const QString &alt_key)
{
    ListViewItemLirc *item = new ListViewItemLirc(m_ActionList, m_ActionList->lastChild());
    if (item) {
        QObject::connect(item, SIGNAL(sigRenamingStarted (ListViewItemLirc *, int)),
                         this, SLOT  (slotRenamingStarted(ListViewItemLirc *, int)));
        QObject::connect(item, SIGNAL(sigRenamingStopped (ListViewItemLirc *, int)),
                         this, SLOT  (slotRenamingStopped(ListViewItemLirc *, int)));
        item->setText(0, descr);
        item->setText(1, key);
        item->setText(2, alt_key);
        item->setRenameEnabled(1, true);
        item->setRenameEnabled(2, true);
    }
}

void LIRCConfiguration::slotUpdateConfig()
{
    slotSetDirty();
    slotCancel();
}

void LIRCConfiguration::slotRawLIRCSignal(const QString &val, int /*repeat_counter*/, bool &consumed)
{
    QListViewItem *_it = m_ActionList->currentItem();
    ListViewItemLirc *it = static_cast<ListViewItemLirc*>(_it);
    if (it->isRenamingInProcess()) {
        int col = it->getRenamingColumn();
        it->cancelRename(col);
        it->setText(col, val);
        consumed = true;
        m_dirty = true;
    }
}

void LIRCConfiguration::slotRenamingStarted(ListViewItemLirc */*sender*/, int /*col*/)
{
    m_LabelHints->setText(i18n("Enter the key string of your remote or just press the button on your remote control"));
}


void LIRCConfiguration::slotRenamingStopped(ListViewItemLirc */*sender*/, int /*col*/)
{
    m_LabelHints->setText(i18n("Double Click on the entries to change the assignments"));
}


void LIRCConfiguration::slotSetDirty()
{
    if (!m_ignore_gui_updates) {
        m_dirty = true;
    }
}

#include "lirc-configuration.moc"
