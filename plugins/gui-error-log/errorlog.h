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

#include <kpagedialog.h>
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

    virtual QString pluginClassName() const { return "ErrorLog"; }
//     virtual const QString &name() const { return PluginBase::name(); }
//     virtual       QString &name()       { return PluginBase::name(); }

    virtual bool connectI (Interface *);
    virtual bool disconnectI (Interface *);

// WidgetPluginBase

    virtual void   saveState (KConfigGroup &) const;
    virtual void   restoreState (const KConfigGroup &);
    virtual void   restoreState (const KConfigGroup &g, bool b) { WidgetPluginBase::restoreState(g, b); }

public slots:
    virtual void     toggleShown () { WidgetPluginBase::pToggleShown(); }

public:
    virtual void     setVisible(bool v);

protected:
                  QWidget *getWidget()       { return this; }
            const QWidget *getWidget() const { return this; }

    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);

    struct Page {
        QMutex mutex;
        KTextEdit *edit;
        KPageWidgetItem *item;
    };

    void setTextEditPage(const QString &title, const KIcon &icon, Page *page);
    void logToPage(Page &page, const QString &text, bool showUp = false);

// IErrorLog

RECEIVERS:
    bool logError  (const QString &);
    bool logWarning(const QString &);
    bool logInfo   (const QString &);
    bool logDebug  (const QString &);

// KDialogBase

protected slots:

    void slotUser1();

protected:

    Page             m_pageInfo,
                     m_pageWarnings,
                     m_pageErrors,
                     m_pageDebug;

    bool             init_done;
};

#endif
