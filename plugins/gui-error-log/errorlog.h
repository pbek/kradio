/***************************************************************************
                          errorlog.h  -  description
                             -------------------
    begin                : Sa Sep 13 2003
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

#ifndef KRADIO_ERRORLOG_H
#define KRADIO_ERRORLOG_H

#include <KPageDialog>
#include <QMutex>

#include "errorlog_interfaces.h"
#include "widgetpluginbase.h"


class KTextEdit;
class ErrorLog : public KPageDialog,
                 public WidgetPluginBase,
                 public IErrorLog
{
Q_OBJECT
public:
    ErrorLog(const QString &instanceID, const QString &name);
    ~ErrorLog();

    virtual QString pluginClassName() const override { return QString::fromLatin1("ErrorLog"); }

    virtual bool connectI (Interface *)    override;
    virtual bool disconnectI (Interface *) override;

// WidgetPluginBase

    virtual void   saveState (KConfigGroup &) const    override;
    virtual void   restoreState (const KConfigGroup &) override;
    virtual void   restoreState (const KConfigGroup &g, bool b) override { WidgetPluginBase::restoreState(g, b); }

public:
    virtual void     setVisible(bool v) override;

protected:
                  QWidget *getWidget()       override { return this; }
            const QWidget *getWidget() const override { return this; }

    virtual void showEvent(QShowEvent *) override;
    virtual void hideEvent(QHideEvent *) override;

    struct Page {
        QMutex            mutex;
        KTextEdit       * edit;
        KPageWidgetItem * item;
    };

    void setTextEditPage(const QString &title, const QIcon &icon, Page *page);
    void logToPage(Page &page, const QString &text, bool showUp = false);

// IErrorLog

RECEIVERS:
    bool logError  (const QString &) override;
    bool logWarning(const QString &) override;
    bool logInfo   (const QString &) override;
    bool logDebug  (const QString &) override;

// KDialogBase

protected slots:

    void slotSaveAs();

protected:

    Page             m_pageInfo,
                     m_pageWarnings,
                     m_pageErrors,
                     m_pageDebug;

    bool             init_done;
};

#endif
