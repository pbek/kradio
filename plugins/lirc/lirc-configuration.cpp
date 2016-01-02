/***************************************************************************
                       lirc-configuration.cpp  -  description
                             -------------------
    begin                : Sat May 21 2005
    copyright            : (C) 2005 by Martin Witte
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

#include <QLabel>
#include <QString>
#include <QFileInfo>
#include <QWidget>
#include <QStandardItemModel>

#include <klocalizedstring.h>
#include <kurlrequester.h>
#include <kdemacros.h>

#include <lirc/lirc_client.h>

#include "lirc-configuration.h"
#include "lircsupport.h"
#include "styleditemdelegate_lirc.h"

LIRCConfiguration::LIRCConfiguration (QWidget *parent, LircSupport *dev)
 : QWidget(parent),
   m_LIRC (dev),
   m_dirty(true),
   m_ignore_gui_updates(false)
{

    setupUi(this);
    edLIRCConfigurationFile->setMode(KFile::LocalOnly | KFile::File);

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

    m_model = new QStandardItemModel(m_ActionList);
    QStringList headers;
    headers.append(i18n("Action"));
    headers.append(i18n("LIRC String"));
    headers.append(i18n("Alternative LIRC String"));
    m_model->setHorizontalHeaderLabels(headers);
    m_ActionList->setModel(m_model);
    m_delegate = new StyledItemDelegateLirc(m_ActionList);
    m_ActionList->setItemDelegate(m_delegate);

    connect(m_model,           SIGNAL(itemChanged(QStandardItem*)),       this, SLOT(slotSetDirty()));
    connect(comboPowerOffMode, SIGNAL(currentIndexChanged(int)),          this, SLOT(slotSetDirty()));
    connect(comboPowerOnMode,  SIGNAL(currentIndexChanged(int)),          this, SLOT(slotSetDirty()));
    connect(cbSyncAtRuntime,   SIGNAL(toggled(bool)),                     this, SLOT(slotSetDirty()));
    connect(cbSyncAtStartup,   SIGNAL(toggled(bool)),                     this, SLOT(slotSetDirty()));
    connect(m_delegate, SIGNAL(startedRenaming(QModelIndex)), this, SLOT(slotRenamingStarted(QModelIndex)));
    connect(m_delegate, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
            this, SLOT(slotRenamingStopped()));

    connect(edLIRCConfigurationFile, SIGNAL(textChanged(const QString &)), this, SLOT(readLIRCConfigurationFile()));
    connect(edLIRCConfigurationFile, SIGNAL(textChanged(const QString &)), this, SLOT(slotSetDirty()));
    slotRenamingStopped();
    slotCancel();
}


LIRCConfiguration::~LIRCConfiguration ()
{
}

void LIRCConfiguration::readLIRCConfigurationFile()
{
    comboPowerOffMode->clear();
    comboPowerOnMode ->clear();
    comboPowerOffMode->addItem(i18n("<do not care>"), QVariant(""));
    comboPowerOnMode ->addItem(i18n("<do not care>"), QVariant(""));

    if (m_LIRC && m_LIRC->getLIRC_fd() >= 0) {

        QStringList modes;

        QString     lirc_config_file    = edLIRCConfigurationFile->url().path();
        QByteArray  lirc_cfg_filename_ba = QFile::encodeName(lirc_config_file);

        QFileInfo   lirc_config_file_info(lirc_config_file);

        struct lirc_config *cfg = NULL;

    //     IErrorLogClient::staticLogDebug(QString("LIRCConfiguration::readLIRCConfigurationFile: lirc config file = >%1<").arg(lirc_config_file));
    //     IErrorLogClient::staticLogDebug(QString("LIRCConfiguration::readLIRCConfigurationFile: lirc config file(C-String) = >%1<").arg(lirc_cfg_filename_ba.constData()));

        if (lirc_config_file_info.isFile() && lirc_config_file_info.exists() && lirc_readconfig (lirc_cfg_filename_ba.data(), &cfg, NULL) == 0) {

            for (lirc_config_entry *e = cfg ? cfg->first : NULL; e; e = e->next) {
                QString mode = e->mode;
                if (mode.length() && modes.indexOf(mode) < 0) {
                    modes.append(mode);
                }
            }
            modes.sort();

            QString mode;
            foreach (mode, modes) {
                comboPowerOffMode->addItem(mode, QVariant(mode));
                comboPowerOnMode ->addItem(mode, QVariant(mode));
            }
        }
    }
}


void LIRCConfiguration::slotOK()
{
    if (m_dirty && m_LIRC) {
        m_LIRC->setLIRCConfigurationFile(edLIRCConfigurationFile->url().path());

        QMap<LIRC_Actions, QString> actions;
        QMap<LIRC_Actions, QString> alt_actions;

        const int rowCount = m_model->rowCount();
        for (int i = 0; i < rowCount; ++i) {
            LIRC_Actions action = m_order[i];
            actions[action]     = m_model->item(i, 1)->text();
            alt_actions[action] = m_model->item(i, 2)->text();
        }
        m_LIRC->setActions(actions, alt_actions);

        m_LIRC->setLIRCModeSync(cbSyncAtStartup->isChecked(), cbSyncAtRuntime->isChecked());
        m_LIRC->setPowerOnMode (comboPowerOnMode ->itemData(comboPowerOnMode ->currentIndex()).toString());
        m_LIRC->setPowerOffMode(comboPowerOffMode->itemData(comboPowerOffMode->currentIndex()).toString());
    }
    m_dirty = false;
}


void LIRCConfiguration::slotCancel()
{
    if (m_dirty) {
        m_ignore_gui_updates = true;
        m_delegate->stopEditing();
        // do not use clear() with QStandardItemModel, as it removes
        // also the header items!
        m_model->setRowCount(0);

        int idx_power_on  = -1;
        int idx_power_off = -1;
        bool at_startup = false;
        bool at_runtime = false;

        if (m_LIRC) {

            edLIRCConfigurationFile->setStartDir(m_LIRC->getLIRCConfigurationFile());
            readLIRCConfigurationFile();

            const QMap<LIRC_Actions, QString> &actions     = m_LIRC->getActions();
            const QMap<LIRC_Actions, QString> &alt_actions = m_LIRC->getAlternativeActions();

            for (int i = 0; m_order.contains(i) && i < m_order.count(); ++i) {
                LIRC_Actions action = m_order[i];
                addKey(m_descriptions[action], actions[action], alt_actions[action]);
            }
            m_ActionList->resizeColumnToContents(0);
            m_ActionList->resizeColumnToContents(1);
            m_ActionList->resizeColumnToContents(2);

            idx_power_on  = comboPowerOnMode ->findData(m_LIRC->getPowerOnMode());
            idx_power_off = comboPowerOffMode->findData(m_LIRC->getPowerOffMode());
            m_LIRC->getLIRCModeSync(at_startup, at_runtime);
        }

        comboPowerOnMode ->setCurrentIndex(idx_power_on  < 0 ? 0 : idx_power_on);
        comboPowerOffMode->setCurrentIndex(idx_power_off < 0 ? 0 : idx_power_off);

        cbSyncAtStartup->setChecked(at_startup);
        cbSyncAtRuntime->setChecked(at_runtime);

        m_ignore_gui_updates = false;
    }
    m_dirty = false;
}


void LIRCConfiguration::addKey(const QString &descr, const QString &key, const QString &alt_key)
{
    QStandardItem *item;
    QList<QStandardItem *> row;
    item = new QStandardItem(descr);
    item->setEditable(false);
    row.append(item);
    item = new QStandardItem(key);
    row.append(item);
    item = new QStandardItem(alt_key);
    row.append(item);
    m_model->appendRow(row);
}

void LIRCConfiguration::slotUpdateConfig()
{
    slotSetDirty();
    slotCancel();
}

void LIRCConfiguration::slotRawLIRCSignal(const QString &val, int /*repeat_counter*/, bool &consumed)
{
    if (m_renamingItem.isValid()) {
        // keep a copy of m_renamingItem before canceling editing
        const QModelIndex index = m_renamingItem;
        m_delegate->stopEditing();
        m_model->setData(index, val);
        consumed = true;
        m_dirty = true;
    }
}

void LIRCConfiguration::slotRenamingStarted(const QModelIndex &index)
{
    m_renamingItem = index;
    m_LabelHints->setText(i18n("Enter the key string of your remote or just press the button on your remote control"));
}


void LIRCConfiguration::slotRenamingStopped()
{
    m_renamingItem = QPersistentModelIndex();
    m_LabelHints->setText(i18n("Double click on the entries to change the assignments"));
}


void LIRCConfiguration::slotSetDirty()
{
    if (!m_ignore_gui_updates) {
        m_dirty = true;
    }
}

#include "lirc-configuration.moc"
