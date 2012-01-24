/***************************************************************************
                          errorlog.cpp  -  description
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

#include "errorlog.h"

#include <QtGui/QFrame>
#include <QtCore/QDateTime>
#include <QtGui/QLayout>
#include <QtCore/QTextCodec>

#include <ktextedit.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <ktemporaryfile.h>
#include <kio/netaccess.h>

///////////////////////////////////////////////////////////////////////

PLUGIN_LIBRARY_FUNCTIONS(ErrorLog, PROJECT_NAME, i18n("Error Logging Window for KRadio"));

/////////////////////////////////////////////////////////////////////////////

ErrorLog::ErrorLog(const QString &instanceID, const QString &name)
    :  KPageDialog(),
       WidgetPluginBase(this, instanceID, name, i18n("Error Logger")),
       m_debug(NULL),
       m_infos(NULL),
       m_warnings(NULL),
       m_errors(NULL),
       init_done(false)
{
    setFaceType        (KPageDialog::List);
    setCaption         (i18n("KRadio Logger"));
    setObjectName      (name);
    setModal           (false);
    setButtons         (KDialog::Close | KDialog::User1);
    setDefaultButton   (KDialog::Close);
    showButtonSeparator(true);
    setButtonGuiItem(KDialog::User1, KGuiItem(i18n("Save &as"), KIcon("document-save-as")));


    setWindowTitle(i18n("KRadio Logger"));

    m_teInfos = createTextEditPage(i18n("Information"), KIcon("dialog-information"), m_infos);
    logInfo(i18n("KRadio4 Version %1 logging started", QString(KRADIO_VERSION)));

    m_teWarnings = createTextEditPage(i18n("Warnings"), KIcon("dialog-warning"), m_warnings);
    logWarning(i18n("KRadio4 Version %1 logging started", QString(KRADIO_VERSION)));

    m_teErrors = createTextEditPage(i18n("Errors"), KIcon("dialog-error"), m_errors);
    logError(i18n("KRadio4 Version %1 logging started", QString(KRADIO_VERSION)));

    m_teDebug = createTextEditPage(i18n("Debugging"), KIcon("system-search"), m_debug);
    logDebug(i18n("KRadio4 Version %1 logging started", QString(KRADIO_VERSION)));

    QObject::connect(this, SIGNAL(user1Clicked()), this, SLOT(slotUser1()));

    init_done = true;
}


ErrorLog::~ErrorLog()
{
}


KTextEdit *ErrorLog::createTextEditPage(const QString &title, const KIcon &icon, KPageWidgetItem *&item)
{
    QFrame      *frame = new QFrame(this);
    QGridLayout *linfo = new QGridLayout(frame);
    KTextEdit   *edit  = new KTextEdit(frame);
    linfo->setSpacing( 5 );
    linfo->setMargin ( 0 );
    linfo->addWidget(edit, 0, 0);
    edit->setReadOnly(true);

    item = addPage(frame, title);
    item->setHeader(title);
    item->setIcon(icon);

    return edit;
}


bool ErrorLog::connectI (Interface *i)
{
    bool a = IErrorLog::connectI(i);
    bool b = PluginBase::connectI(i);
    return a || b;
}

bool ErrorLog::disconnectI (Interface *i)
{
    bool a = IErrorLog::disconnectI(i);
    bool b = PluginBase::disconnectI(i);
    return a || b;
}

void ErrorLog::restoreState (const KConfigGroup &config)
{
    WidgetPluginBase::restoreState(config, false);
}


void ErrorLog::saveState (KConfigGroup &config) const
{
    WidgetPluginBase::saveState(config);
}


void ErrorLog::setVisible(bool v)
{
    pSetVisible(v);
    KPageDialog::setVisible(v);
}

void    ErrorLog::showEvent(QShowEvent *e)
{
    KPageDialog::showEvent(e);
    WidgetPluginBase::pShowEvent(e);
}

void    ErrorLog::hideEvent(QHideEvent *e)
{
    KPageDialog::hideEvent(e);
    WidgetPluginBase::pHideEvent(e);
}

// IErrorLog

bool ErrorLog::logError  (const QString &s)
{
    QMutexLocker   lock(&m_sequentializer);
    m_teErrors->append("<i>" + QDateTime::currentDateTime().toString(Qt::ISODate) + "</i> " + s + "\n");
    if (init_done) {
        setCurrentPage(m_errors);
        show();
    }
    return true;
}

bool ErrorLog::logWarning(const QString &s)
{
    QMutexLocker   lock(&m_sequentializer);
    m_teWarnings->append("<i>" + QDateTime::currentDateTime().toString(Qt::ISODate) + "</i> " + s + "\n");
    return true;
}

bool ErrorLog::logInfo   (const QString &s)
{
    QMutexLocker   lock(&m_sequentializer);
    m_teInfos->append("<i>" + QDateTime::currentDateTime().toString(Qt::ISODate) + "</i> " + s + "\n");
    return true;
}

bool ErrorLog::logDebug   (const QString &s)
{
    QMutexLocker   lock(&m_sequentializer);
    m_teDebug->append("<i>" + QDateTime::currentDateTime().toString(Qt::ISODate) + "</i> " + s + "\n");
    return true;
}

// KDialogBase


// store Log Data
void ErrorLog::slotUser1()
{
    KFileDialog fd(KUrl(),
                   ("*.log|" + i18n("Log Files") + "( *.log )"),
                   this);
    fd.setModal(true);
    fd.setMode(KFile::File);
    fd.setOperationMode(KFileDialog::Saving);
    fd.setWindowTitle (i18n("Save KRadio Logging Data as ..."));

    if (fd.exec() == QDialog::Accepted) {
        KUrl url = fd.selectedUrl();

        KTemporaryFile tmpFile;
        tmpFile.setAutoRemove(true);
        if (tmpFile.open()) {
            QString filename = tmpFile.fileName();

            QTextStream outs(&tmpFile);
            outs.setCodec(QTextCodec::QTextCodec::codecForName("UTF-8"));

            if (currentPage() == m_errors) {
                outs << m_teErrors->toPlainText();
            } else if (currentPage() == m_warnings) {
                outs << m_teWarnings->toPlainText();
            } else if (currentPage() == m_infos) {
                outs << m_teInfos->toPlainText();
            } else if (currentPage() == m_debug) {
                outs << m_teDebug->toPlainText();
            }

            if (tmpFile.error()) {
                logError("ErrorLogger: " +
                        i18n("error writing to tempfile %1", filename));
                return;
            }

            // close hopefully flushes buffers ;)
            tmpFile.close();

            if (!KIO::NetAccess::upload(filename, url, this)) {
                logError("ErrorLogger: " +
                        i18n("error uploading preset file %1", url.pathOrUrl()));
            }
        }
    }
    // FIXME: no idea how to port
    //setIconListAllVisible(true);
}


#include "errorlog.moc"
