/***************************************************************************
                          errorlog.cpp  -  description
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

#include "errorlog.h"

#include <qframe.h>
#include <qdatetime.h>
#include <qlayout.h>

#include <klocale.h>
#include <kiconloader.h>
#include <ktextedit.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <ktempfile.h>
#include <kio/netaccess.h>

#define PAGE_ID_INFO  0
#define PAGE_ID_WARN  1
#define PAGE_ID_ERROR 2
#define PAGE_ID_DEBUG 3

ErrorLog::ErrorLog(QWidget * parent, const char * name)
	:  KDialogBase(KDialogBase::IconList,
	               i18n("KRadio Logger"),
	               KDialogBase::Close|KDialogBase::User1,
	               KDialogBase::Close,
	               parent,
	               name,
	               false,
	               false,
	               KGuiItem(i18n("Save &as"), "filesaveas")
	               ),
	   WidgetPluginBase(name, i18n("Error Logger")),
	   init_done(false)
{
	IErrorLog::connect();

	QFrame *info = addPage(i18n("Information"), i18n("Information"),
	                       KGlobal::instance()->iconLoader()->loadIcon(
	                          "messagebox_info", KIcon::NoGroup, KIcon::SizeMedium
	                       )
	                      );

    QGridLayout *linfo = new QGridLayout(info);
    linfo->setSpacing( 5 );
    linfo->setMargin ( 0 );
    m_teInfos = new KTextEdit(info);
    linfo->addWidget(m_teInfos, 0, 0);
    m_teInfos->setReadOnly(true);
    logInfo(i18n("logging started"));

	                      
	QFrame *warn = addPage(i18n("Warnings"),    i18n("Warnings"),
	                       KGlobal::instance()->iconLoader()->loadIcon(
	                          "messagebox_warning", KIcon::NoGroup, KIcon::SizeMedium
	                       )
	                      );
    QGridLayout *lwarn = new QGridLayout(warn);
    lwarn->setSpacing( 5 );
    lwarn->setMargin ( 0 );
    m_teWarnings = new KTextEdit(warn);
    lwarn->addWidget(m_teWarnings, 0, 0);
    m_teWarnings->setReadOnly(true);
    logWarning(i18n("logging started"));



	QFrame *err  = addPage(i18n("Errors"),      i18n("Errors"),
	                       KGlobal::instance()->iconLoader()->loadIcon(
	                          "messagebox_critical", KIcon::NoGroup, KIcon::SizeMedium
	                       )
	                      );
    QGridLayout *lerr = new QGridLayout(err);
    lerr->setSpacing( 5 );
    lerr->setMargin ( 0 );
    m_teErrors = new KTextEdit(err);
    lerr->addWidget(m_teErrors, 0, 0);
    m_teErrors->setReadOnly(true);
    logError(i18n("logging started"));

	QFrame *debug = addPage(i18n("Debugging"), i18n("Debugging"),
	                       KGlobal::instance()->iconLoader()->loadIcon(
	                          "find", KIcon::NoGroup, KIcon::SizeMedium
	                       )
	                      );

    QGridLayout *ldebug = new QGridLayout(debug);
    ldebug->setSpacing( 5 );
    ldebug->setMargin ( 0 );
    m_teDebug = new KTextEdit(debug);
    ldebug->addWidget(m_teDebug, 0, 0);
    m_teDebug->setReadOnly(true);
    logDebug(i18n("logging started"));

    init_done = true;
}


ErrorLog::~ErrorLog()
{
}


void ErrorLog::restoreState (KConfig *config)
{
    config->setGroup(QString("errorlog-") + WidgetPluginBase::name());
	WidgetPluginBase::restoreState(config, false);
}

 
void ErrorLog::saveState (KConfig *config) const
{
    config->setGroup(QString("errorlog-") + WidgetPluginBase::name());
	WidgetPluginBase::saveState(config);
}


void ErrorLog::show()
{
    WidgetPluginBase::show();
	KDialogBase::show();
}

void ErrorLog::hide()
{
    WidgetPluginBase::hide();
    KDialogBase::hide();
}

void    ErrorLog::showEvent(QShowEvent *e)
{
	KDialogBase::showEvent(e);
	WidgetPluginBase::showEvent(e);
}

void    ErrorLog::hideEvent(QHideEvent *e)
{
	KDialogBase::hideEvent(e);
	WidgetPluginBase::hideEvent(e);
}

// IErrorLog

bool ErrorLog::logError  (const QString &s)
{
	m_teErrors->append("<i>" + QDateTime::currentDateTime().toString(Qt::ISODate) + "</i> " + s + "\n");
    if (init_done) {
		showPage(PAGE_ID_ERROR);
		show();
	}
	return true;
}

bool ErrorLog::logWarning(const QString &s)
{
	m_teWarnings->append("<i>" + QDateTime::currentDateTime().toString(Qt::ISODate) + "</i> " + s + "\n");
	return true;
}

bool ErrorLog::logInfo   (const QString &s)
{
	m_teInfos->append("<i>" + QDateTime::currentDateTime().toString(Qt::ISODate) + "</i> " + s + "\n");
	return true;
}

bool ErrorLog::logDebug   (const QString &s)
{
	m_teDebug->append("<i>" + QDateTime::currentDateTime().toString(Qt::ISODate) + "</i> " + s + "\n");
	return true;
}

// KDialogBase


// store Log Data
void ErrorLog::slotUser1()
{
    KFileDialog fd("",
		           "*.log|" + i18n("Log Files") + "( *.log )",
		           this,
		           i18n("Select Log File"),
		           true);
    fd.setMode(KFile::File);
    fd.setOperationMode(KFileDialog::Saving);
    fd.setCaption (i18n("Save KRadio Logging Data as ..."));

    if (fd.exec() == QDialog::Accepted) {
		KURL url = fd.selectedURL();

		KTempFile tmpFile;
		tmpFile.setAutoDelete(true);
		QFile *outf = tmpFile.file();

		QTextStream outs(outf);
		outs.setEncoding(QTextStream::UnicodeUTF8);

		switch (activePageIndex()) {
			case PAGE_ID_INFO:  outs << m_teInfos->text();    break;
			case PAGE_ID_WARN:  outs << m_teWarnings->text(); break;
			case PAGE_ID_ERROR: outs << m_teErrors->text();   break;
			case PAGE_ID_DEBUG: outs << m_teDebug->text();    break;
			default: break;
		}
		
		if (outf->status() != IO_Ok) {
			logError("ErrorLogger: " +
				     i18n("error writing to tempfile %1").arg(tmpFile.name()));
			return;
		}

		// close hopefully flushes buffers ;)
		outf->close();

		if (!KIO::NetAccess::upload(tmpFile.name(), url)) {
			logError("ErrorLogger: " +
		             i18n("error uploading preset file %1").arg(url.url()));
		}
    }
    setIconListAllVisible(true);
}
