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

#include <QtWidgets/QLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>

#include <QtCore/QDateTime>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QUrl>
#include <QtCore/QTemporaryFile>

#include <QtGui/QTextDocument>
#include <QtGui/QIcon>


#include <KTextEdit>
#include <KLocalizedString>

#include "kio_put_wrapper.h"

///////////////////////////////////////////////////////////////////////

static KAboutData aboutData()
{
    KAboutData about("ErrorLog",
                     NULL,
                     KRADIO_VERSION,
                     i18nc("@title", "Error Logging Window"),
                     KAboutLicense::LicenseKey::GPL,
                     NULL,
                     NULL,
                     "http://sourceforge.net/projects/kradio",
                     "emw-kradio@nocabal.de");
    return about;
}

KRADIO_EXPORT_PLUGIN(ErrorLog, aboutData())
#include "errorlog.moc"

/////////////////////////////////////////////////////////////////////////////

ErrorLog::ErrorLog(const QString &instanceID, const QString &name)
    :  KPageDialog(),
       WidgetPluginBase(this, instanceID, name, i18n("Error Logger")),
       init_done(false)
{
    setFaceType        (KPageDialog::List);
    setWindowTitle     (i18n("KRadio Logger"));
    setObjectName      (name);
    setModal           (false);
    setStandardButtons (QDialogButtonBox::Close);
    buttonBox()->button(QDialogButtonBox::Close)->setDefault(true);
    
    QPushButton *saveAsButton = new QPushButton(QIcon("document-save-as"), i18n("Save &as"));
    buttonBox()->addButton(saveAsButton, QDialogButtonBox::InvalidRole);

    setWindowTitle(i18n("KRadio Logger"));

    setTextEditPage(i18n("Information"), QIcon("dialog-information"), &m_pageInfo);
    logInfo(i18n("KRadio5 Version %1 logging started", QString(KRADIO_VERSION)));

    setTextEditPage(i18n("Warnings"), QIcon("dialog-warning"), &m_pageWarnings);
    logWarning(i18n("KRadio5 Version %1 logging started", QString(KRADIO_VERSION)));

    setTextEditPage(i18n("Errors"), QIcon("dialog-error"), &m_pageErrors);
    logError(i18n("KRadio5 Version %1 logging started", QString(KRADIO_VERSION)));

    setTextEditPage(i18n("Debugging"), QIcon("system-search"), &m_pageDebug);
    logDebug(i18n("KRadio5 Version %1 logging started", QString(KRADIO_VERSION)));

    QObject::connect(saveAsButton, &QPushButton::clicked, this, &ErrorLog::slotSaveAs);

    init_done = true;
}


ErrorLog::~ErrorLog()
{
}


void ErrorLog::setTextEditPage(const QString &title, const QIcon &icon, Page *page)
{
    QWidget     *frame = new QWidget(this);
    QGridLayout *linfo = new QGridLayout(frame);
    page->edit         = new KTextEdit(frame);
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
    const QString escaped = text.toHtmlEscaped();
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
    logToPage(m_pageErrors, s, true);
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
void ErrorLog::slotSaveAs()
{
    const QUrl url = QFileDialog::getSaveFileUrl(this, i18n("Save KRadio Logging Data"), QUrl(), i18n("Log Files") + " (*.log)");

    if (url.isValid()) {
        QTemporaryFile tmpFile(this);
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
            
            tmpFile.open();            
            
            const QByteArray tmpFileData = tmpFile.readAll();
            tmpFile.close();
            
            kio_put_wrapper_t  kio_put_wrapper(url, tmpFileData);
            
            if (!kio_put_wrapper.ok()) {
                logError("ErrorLogger: " +
                        i18n("error uploading preset file %1", url.toString()));
            }
        }
    }
    // FIXME: no idea how to port
    //setIconListAllVisible(true);
}


