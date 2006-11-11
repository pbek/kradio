/***************************************************************************
                          errorlog.h  -  description
                             -------------------
    begin                : Sa Sep 13 2003
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

#ifndef KRADIO_ERRORLOG_H
#define KRADIO_ERRORLOG_H

#include <kdialogbase.h>

#include "../../src/include/errorlog-interfaces.h"
#include "../../src/include/widgetplugins.h"


class QTextEdit;
class ErrorLog : public KDialogBase,
                 public WidgetPluginBase,
                 public IErrorLog
{
Q_OBJECT
public:
    ErrorLog(const QString &name = QString::null);
    ~ErrorLog();

    virtual QString pluginClassName() const { return "ErrorLog"; }
    virtual const QString &name() const { return PluginBase::name(); }
    virtual       QString &name()       { return PluginBase::name(); }

    virtual bool connectI (Interface *);
    virtual bool disconnectI (Interface *);

// WidgetPluginBase

    virtual void   saveState (KConfig *) const;
    virtual void   restoreState (KConfig *);

public slots:
    virtual void     showOnOrgDesktop();
    virtual void     show();
    virtual void     hide();
    virtual void     toggleShown () { WidgetPluginBase::pToggleShown(); }

protected:
                  QWidget *getWidget()       { return this; }
            const QWidget *getWidget() const { return this; }

    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);

    virtual ConfigPageInfo createConfigurationPage () { return ConfigPageInfo(); }
    virtual AboutPageInfo  createAboutPage ()         { return AboutPageInfo(); }

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

    QTextEdit  *m_teDebug,
               *m_teInfos,
               *m_teWarnings,
               *m_teErrors;

    bool        init_done;
};

#endif
