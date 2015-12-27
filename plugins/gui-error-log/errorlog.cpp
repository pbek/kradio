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

#include <QDateTime>
#include <QLayout>
#include <QTextCodec>
#include <QTextStream>
#include <QTextDocument>

#include <ktextedit.h>
#include <klocalizedstring.h>
#include <kicon.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <ktemporaryfile.h>
#include <kio/netaccess.h>

///////////////////////////////////////////////////////////////////////

static KAboutData aboutData()
{
    KAboutData about("ErrorLog",
                     PROJECT_NAME,
                     KLocalizedString(),
                     KRADIO_VERSION,
                     ki18nc("@title", "Error Logging Window"),
                     KAboutData::License_GPL,
                     KLocalizedString(),
                     KLocalizedString(),
                     "http://sourceforge.net/projects/kradio",
                     "emw-kradio@nocabal.de");
    return about;
}

KRADIO_EXPORT_PLUGIN(ErrorLog, aboutData())

/////////////////////////////////////////////////////////////////////////////

ErrorLog::ErrorLog(const QString &instanceID, const QString &name)
    :  KPageDialog(),
       WidgetPluginBase(this, instanceID, name, i18n("Error Logger")),
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

    setTextEditPage(i18n("Information"), KIcon("dialog-information"), &m_pageInfo);
    logInfo(i18n("KRadio4 Version %1 logging started", QString(KRADIO_VERSION)));

    setTextEditPage(i18n("Warnings"), KIcon("dialog-warning"), &m_pageWarnings);
    logWarning(i18n("KRadio4 Version %1 logging started", QString(KRADIO_VERSION)));

    setTextEditPage(i18n("Errors"), KIcon("dialog-error"), &m_pageErrors);
    logError(i18n("KRadio4 Version %1 logging started", QString(KRADIO_VERSION)));

    setTextEditPage(i18n("Debugging"), KIcon("system-search"), &m_pageDebug);
    logDebug(i18n("KRadio4 Version %1 logging started", QString(KRADIO_VERSION)));

    QObject::connect(this, SIGNAL(user1Clicked()), this, SLOT(slotUser1()));

    init_done = true;
}


ErrorLog::~ErrorLog()
{
}


void ErrorLog::setTextEditPage(const QString &title, const KIcon &icon, Page *page)
{
    QWidget     *frame = new QWidget(this);
    QGridLayout *linfo = new QGridLayout(frame);
    page->edit         = new KTextEdit(frame);
    linfo->setSpacing( 5 );
    linfo->setMargin ( 0 );
    linfo->addWidget(page->edit, 0, 0);
    page->edit->setReadOnly(true);
    page->edit->setUndoRedoEnabled(false);

    page->item = addPage(frame, title);
    page->item->setHeader(title);
    page->item->setIcon(icon);
}

void ErrorLog::logToPage(Page &page, const QString &text, bool showUp)
{
    const QString escaped = Qt::escape(text);
    const QString msg = "<i>" + QDateTime::currentDateTime().toString(Qt::ISODate) + "</i> " + escaped + "\n";
    QMutexLocker lock(&page.mutex);
    page.edit->append(msg);
    if (showUp && init_done) {
        setCurrentPage(page.item);
        show();
    }
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
    logToPage(m_pageInfo, s, true);
    return true;
}

bool ErrorLog::logWarning(const QString &s)
{
    logToPage(m_pageWarnings, s);
    return true;
}

bool ErrorLog::logInfo   (const QString &s)
{
    logToPage(m_pageInfo, s);
    return true;
}

bool ErrorLog::logDebug   (const QString &s)
{
    logToPage(m_pageDebug, s);
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
    fd.setWindowTitle (i18n("Save KRadio Logging Data"));

    if (fd.exec() == QDialog::Accepted) {
        KUrl url = fd.selectedUrl();

        KTemporaryFile tmpFile;
        tmpFile.setAutoRemove(true);
        if (tmpFile.open()) {
            QString filename = tmpFile.fileName();

            QTextStream outs(&tmpFile);
            outs.setCodec(QTextCodec::QTextCodec::codecForName("UTF-8"));

            if (currentPage() == m_pageErrors.item) {
                outs << m_pageErrors.edit->toPlainText();
            } else if (currentPage() == m_pageWarnings.item) {
                outs << m_pageWarnings.edit->toPlainText();
            } else if (currentPage() == m_pageInfo.item) {
                outs << m_pageInfo.edit->toPlainText();
            } else if (currentPage() == m_pageDebug.item) {
                outs << m_pageDebug.edit->toPlainText();
            }

            if (tmpFile.error()) {
                logError("ErrorLogger: " +
                        i18n("error writing to temporary file %1", filename));
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
