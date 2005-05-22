/***************************************************************************
                          aboutwidget.cpp  -  description
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

/* Unfortunately KDE doesn't provide the class KAboutContainerBase
   to public programming, so we have to copy most of that code into
   an own class :(
*/

#include "kradioversion.h"
#include "aboutwidget.h"
#include <qframe.h>
#include <kaboutdialog.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qtabbar.h>
#include <qimage.h>

#include <kglobalsettings.h>
#include <ktextbrowser.h>
#include <qtextedit.h>
#include <kdebug.h>
#include <kapplication.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kurllabel.h>

// copied (and renamed) from kaboutdialog.cpp
// original: KAboutTabWidget

class KRadioAboutTabWidget : public QTabWidget
{
public:
    KRadioAboutTabWidget( QWidget* parent ) : QTabWidget( parent ) {}
    QSize sizeHint() const {
    return QTabWidget::sizeHint().expandedTo( tabBar()->sizeHint() + QSize(4,4) );
    }
};


// copied (renamed and extended) from kaboutdialog.cpp
// original: KAboutContainerBase

KRadioAboutWidget::KRadioAboutWidget(const KAboutData &aboutData, int layoutType, QWidget *_parent,
                      char *_name )
  : QWidget( _parent, _name ),
    mImageLabel(0), mTitleLabel(0), mIconLabel(0),mVersionLabel(0),
    mAuthorLabel(0), mImageFrame(0),mPageTab(0),mPlainSpace(0)
{
  mTopLayout = new QVBoxLayout( this, 0, KDialog::spacingHint() );
  if( mTopLayout == 0 ) { return; }

  if( layoutType & AbtImageOnly )
  {
    layoutType &= ~(AbtImageLeft|AbtImageRight|AbtTabbed|AbtPlain);
  }
  if( layoutType & AbtImageLeft )
  {
    layoutType &= ~AbtImageRight;
  }

  if( layoutType & AbtTitle )
  {
    mTitleLabel = new QLabel( this, "title" );
    mTitleLabel->setAlignment(AlignCenter);
    mTopLayout->addWidget( mTitleLabel );
    mTopLayout->addSpacing( KDialog::spacingHint() );
  }

  if( layoutType & AbtProduct )
  {
    QWidget *productArea = new  QWidget( this, "area" );
    mTopLayout->addWidget( productArea, 0, AlignLeft );

    QHBoxLayout *hbox = new QHBoxLayout(productArea,0,KDialog::spacingHint());
    if( hbox == 0 ) { return; }

    mIconLabel = new QLabel( productArea );
    hbox->addWidget( mIconLabel, 0, AlignLeft|AlignHCenter );

    QVBoxLayout *vbox = new QVBoxLayout();
    if( vbox == 0 ) { return; }
    hbox->addLayout( vbox );

    mVersionLabel = new QLabel( productArea, "version" );
    mAuthorLabel  = new QLabel( productArea, "author" );
    vbox->addWidget( mVersionLabel );
    vbox->addWidget( mAuthorLabel );
    hbox->activate();

    mTopLayout->addSpacing( KDialog::spacingHint() );
  }

  QHBoxLayout *hbox = new QHBoxLayout();
  if( hbox == 0 ) { return; }
  mTopLayout->addLayout( hbox, 10 );

  if( layoutType & AbtImageLeft )
  {
    QVBoxLayout *vbox = new QVBoxLayout();
    hbox->addLayout(vbox);
    vbox->addSpacing(1);
    mImageFrame = new QFrame( this );
    setImageFrame( true );
    vbox->addWidget( mImageFrame );
    vbox->addSpacing(1);

    vbox = new QVBoxLayout( mImageFrame, 1 );
    mImageLabel = new KRadioImageTrackLabel( mImageFrame );
    connect( mImageLabel, SIGNAL(mouseTrack( int, const QMouseEvent * )),
         SLOT( slotMouseTrack( int, const QMouseEvent * )) );
    vbox->addStretch(10);
    vbox->addWidget( mImageLabel );
    vbox->addStretch(10);
    vbox->activate();
  }

  if( layoutType & AbtTabbed )
  {
    mPageTab = new KRadioAboutTabWidget( this );
    if( mPageTab == 0 ) { return; }
    hbox->addWidget( mPageTab, 10 );
  }
  else if( layoutType & AbtImageOnly )
  {
    mImageFrame = new QFrame( this );
    setImageFrame( true );
    hbox->addWidget( mImageFrame, 10 );

    QGridLayout *gbox = new QGridLayout(mImageFrame, 3, 3, 1, 0 );
    gbox->setRowStretch( 0, 10 );
    gbox->setRowStretch( 2, 10 );
    gbox->setColStretch( 0, 10 );
    gbox->setColStretch( 2, 10 );

    mImageLabel = new KRadioImageTrackLabel( mImageFrame );
    connect( mImageLabel, SIGNAL(mouseTrack( int, const QMouseEvent * )),
         SLOT( slotMouseTrack( int, const QMouseEvent * )) );
    gbox->addWidget( mImageLabel, 1, 1 );
    gbox->activate();
  }
  else
  {
    mPlainSpace = new QFrame( this );
    if( mPlainSpace == 0 ) { return; }
    hbox->addWidget( mPlainSpace, 10 );
  }

  if( layoutType & AbtImageRight )
  {
    QVBoxLayout *vbox = new QVBoxLayout();
    hbox->addLayout(vbox);
    vbox->addSpacing(1);
    mImageFrame = new QFrame( this );
    setImageFrame( true );
    vbox->addWidget( mImageFrame );
    vbox->addSpacing(1);

    vbox = new QVBoxLayout( mImageFrame, 1 );
    mImageLabel = new KRadioImageTrackLabel( mImageFrame );
    connect( mImageLabel, SIGNAL(mouseTrack( int, const QMouseEvent * )),
         SLOT( slotMouseTrack( int, const QMouseEvent * )) );
    vbox->addStretch(10);
    vbox->addWidget( mImageLabel );
    vbox->addStretch(10);
    vbox->activate();
  }

  fontChange( font() );

  setAboutData(aboutData);
}


void KRadioAboutWidget::show( void )
{
    QWidget::show();
}

QSize KRadioAboutWidget::sizeHint( void ) const
{
    return minimumSize().expandedTo( QSize( QWidget::sizeHint().width(), 0 ) );
}

void KRadioAboutWidget::fontChange( const QFont &/*oldFont*/ )
{
  if( mTitleLabel != 0 )
  {
    QFont f( KGlobalSettings::generalFont() );
    f.setBold( true );
    f.setPointSize( 14 ); // Perhaps changeable ?
    mTitleLabel->setFont(f);
  }

  if( mVersionLabel != 0 )
  {
    QFont f( KGlobalSettings::generalFont() );
    f.setBold( true );
    mVersionLabel->setFont(f);
    mAuthorLabel->setFont(f);
    mVersionLabel->parentWidget()->layout()->activate();
  }

  update();
}

QFrame *KRadioAboutWidget::addTextPage( const QString &title,
                      const QString &text,
                      bool richText, int numLines )
{
  QFrame *page = addEmptyPage( title );
  if( page == 0 ) { return 0; }
  if( numLines <= 0 ) { numLines = 10; }

  QVBoxLayout *vbox = new QVBoxLayout( page, KDialog::spacingHint() );

  if( richText == true )
  {
    KTextBrowser *browser = new KTextBrowser( page, "browser" );
    browser->setHScrollBarMode( QScrollView::AlwaysOff );
    browser->setText( text );
    browser->setMinimumHeight( fontMetrics().lineSpacing()*numLines );

    vbox->addWidget(browser);
    connect(browser, SIGNAL(urlClick(const QString &)),
        SLOT(slotUrlClick(const QString &)));
    connect(browser, SIGNAL(mailClick(const QString &,const QString &)),
        SLOT(slotMailClick(const QString &,const QString &)));
  }
  else
  {
    QTextEdit *textEdit = new QTextEdit( page, "text" );
    textEdit->setReadOnly( true );
    textEdit->setMinimumHeight( fontMetrics().lineSpacing()*numLines );
    textEdit->setWordWrap( QTextEdit::NoWrap );
    vbox->addWidget( textEdit );
  }

  return page;
}

QFrame *KRadioAboutWidget::addLicensePage( const QString &title,
                      const QString &text, int numLines)
{
  QFrame *page = addEmptyPage( title );
  if( page == 0 ) { return 0; }
  if( numLines <= 0 ) { numLines = 10; }

  QVBoxLayout *vbox = new QVBoxLayout( page, KDialog::spacingHint() );

  QTextEdit *textEdit = new QTextEdit( page, "license" );
  textEdit->setFont( KGlobalSettings::fixedFont() );
  textEdit->setReadOnly( true );
  textEdit->setWordWrap( QTextEdit::NoWrap );
  textEdit->setText( text );
  textEdit->setMinimumHeight( fontMetrics().lineSpacing()*numLines );
  vbox->addWidget( textEdit );

  return page;
}


KAboutContainer *KRadioAboutWidget::addContainerPage( const QString &title,
                            int childAlignment,
                            int innerAlignment )
{
  if( mPageTab == 0 )
  {
    kdDebug(291) << "addPage: " << "Invalid layout" << endl;
    return 0;
  }

  KAboutContainer *container = new KAboutContainer( mPageTab, "container",
    KDialog::spacingHint(), KDialog::spacingHint(), childAlignment,
                          innerAlignment );
  mPageTab->addTab( container, title );

  if( mContainerList.resize( mContainerList.size() + 1) == true )
  {
    mContainerList[ mContainerList.size()-1 ]=container;
  }

  connect(container, SIGNAL(urlClick(const QString &)),
      SLOT(slotUrlClick(const QString &)));
  connect(container, SIGNAL(mailClick(const QString &,const QString &)),
      SLOT(slotMailClick(const QString &,const QString &)));

  return container;
}


KAboutContainer *KRadioAboutWidget::addScrolledContainerPage(
                      const QString &title,
                      int childAlignment,
                      int innerAlignment )
{
  if( mPageTab == 0 )
  {
    kdDebug(291) << "addPage: " << "Invalid layout" << endl;
    return 0;
  }

  QFrame *page = addEmptyPage( title );
  QVBoxLayout *vbox = new QVBoxLayout( page, KDialog::spacingHint() );
  QScrollView *scrollView = new QScrollView( page );
  scrollView->viewport()->setBackgroundMode( PaletteBackground );
  vbox->addWidget( scrollView );

  KAboutContainer *container = new KAboutContainer( scrollView, "container",
    KDialog::spacingHint(), KDialog::spacingHint(), childAlignment,
    innerAlignment );
  scrollView->addChild( container );


  connect(container, SIGNAL(urlClick(const QString &)),
      SLOT(slotUrlClick(const QString &)));
  connect(container, SIGNAL(mailClick(const QString &,const QString &)),
      SLOT(slotMailClick(const QString &,const QString &)));

  return container;
}


QFrame *KRadioAboutWidget::addEmptyPage( const QString &title )
{
  if( mPageTab == 0 )
  {
    kdDebug(291) << "addPage: " << "Invalid layout" << endl;
    return 0;
  }

  QFrame *page = new QFrame( mPageTab, title.latin1() );
  page->setFrameStyle( QFrame::NoFrame );

  mPageTab->addTab( page, title );
  return page;
}


KAboutContainer *KRadioAboutWidget::addContainer( int childAlignment,
                            int innerAlignment )
{
  KAboutContainer *container = new KAboutContainer( this, "container",
    0, KDialog::spacingHint(), childAlignment, innerAlignment );
  mTopLayout->addWidget( container, 0, childAlignment );

  if( mContainerList.resize( mContainerList.size() + 1) == true )
  {
    mContainerList[ mContainerList.size()-1 ]=container;
  }

  connect(container, SIGNAL(urlClick(const QString &)),
      SLOT(slotUrlClick(const QString &)));
  connect(container, SIGNAL(mailClick(const QString &,const QString &)),
      SLOT(slotMailClick(const QString &,const QString &)));

  return container;
}



void KRadioAboutWidget::setTitle( const QString &title )
{
  if( mTitleLabel == 0 )
  {
    kdDebug(291) << "setTitle: " << "Invalid layout" << endl;
    return;
  }
  mTitleLabel->setText(title);
}


void KRadioAboutWidget::setImage( const QString &fileName )
{
  if( mImageLabel == 0 )
  {
    kdDebug(291) << "setImage: " << "Invalid layout" << endl;
    return;
  }
  if( fileName.isNull() )
  {
    return;
  }

  QImage logo( fileName );
  if( logo.isNull() == false )
  {
    QPixmap pix;
    pix = logo;
    mImageLabel->setPixmap( pix );
  }
  mImageFrame->layout()->activate();

}


void KRadioAboutWidget::setImageBackgroundColor( const QColor &color )
{
  if( mImageFrame != 0 )
  {
    mImageFrame->setBackgroundColor( color );
  }
}


void KRadioAboutWidget::setImageFrame( bool state )
{
  if( mImageFrame != 0 )
  {
    if( state == true )
    {
      mImageFrame->setFrameStyle( QFrame::Panel | QFrame::Sunken );
      mImageFrame->setLineWidth(1);
    }
    else
    {
      mImageFrame->setFrameStyle( QFrame::NoFrame );
      mImageFrame->setLineWidth(0);
    }
  }
}


void KRadioAboutWidget::setProduct( const QString &appName,
                      const QString &version,
                      const QString &author,
                      const QString &year )
{
  if( mIconLabel == 0 )
  {
    kdDebug(291) << "setProduct: " << "Invalid layout" << endl;
    return;
  }

  if ( kapp )
      mIconLabel->setPixmap( kapp->icon() );

  // FIXME: Version Numbers should not be part of i18n()
  QString msg1 = i18n("%1 %2 (Using KDE %3)").arg(appName).
    arg(QString::fromLatin1(KRADIO_VERSION)).
    arg(QString::fromLatin1(KDE_VERSION_STRING));
  QString msg2 = !year.isEmpty() ? i18n("%1 %2, %3").arg('©').arg(year).
    arg(author) : QString::fromLatin1("");

  //if (!year.isEmpty())
  //  msg2 = i18n("%1 %2, %3").arg('©').arg(year).arg(author);

  mVersionLabel->setText( msg1 );
  mAuthorLabel->setText( msg2 );
  if( msg2.isEmpty() )
  {
    mAuthorLabel->hide();
  }

  mIconLabel->parentWidget()->layout()->activate();
}


void KRadioAboutWidget::slotMouseTrack( int mode, const QMouseEvent *e )
{
  emit mouseTrack( mode, e );
}


void KRadioAboutWidget::slotUrlClick( const QString &url )
{
    if ( kapp )
        kapp->invokeBrowser( url );
}


void KRadioAboutWidget::slotMailClick( const QString &/*_name*/,
                     const QString &_address )
{
    if ( kapp )
        kapp->invokeMailer( _address, QString::null );
}


// copied (and renamed) from kaboutapplication.cpp
// original: KAboutApplication::buildDialog

void KRadioAboutWidget::setAboutData(const KAboutData &_aboutData)
{
    const KAboutData *aboutData = &_aboutData;
    if (aboutData->programName().length()) {
        setProduct( aboutData->programName(), aboutData->version(),
                    QString::null, QString::null );
    }

    QString appPageText = aboutData->shortDescription() + "\n";

    if (!aboutData->otherText().isEmpty())
        appPageText += "\n" + aboutData->otherText()+"\n";

    if (!aboutData->copyrightStatement().isEmpty())
        appPageText += "\n" + aboutData->copyrightStatement()+"\n";

    KAboutContainer *appPage = addContainerPage( i18n("&About"));

    QLabel *appPageLabel = new QLabel( appPageText, 0 );
    appPage->addWidget( appPageLabel );

    if (!aboutData->homepage().isEmpty()) {
        KURLLabel *url = new KURLLabel();
        url->setText(aboutData->homepage());
        url->setURL(aboutData->homepage());
        appPage->addWidget( url );
        // FIXME
        connect( url, SIGNAL(leftClickedURL(const QString &)),
                 this, SLOT(slotUrlClick(const QString &)));
    }

    int authorCount = aboutData->authors().count();

    if (authorCount) {
        QString authorPageTitle = authorCount == 1 ?
            i18n("A&uthor") : i18n("A&uthors");
        KAboutContainer *authorPage = addScrolledContainerPage( authorPageTitle );
        QValueList<KAboutPerson>::ConstIterator it;
        for (it = aboutData->authors().begin();
            it != aboutData->authors().end(); ++it)
        {
            authorPage->addPerson( (*it).name(), (*it).emailAddress(),
                                   (*it).webAddress(), (*it).task() );
        }
    }

    int creditsCount = aboutData->credits().count();

    if (creditsCount) {
        KAboutContainer *creditsPage =
            addScrolledContainerPage( i18n("&Thanks To") );
        QValueList<KAboutPerson>::ConstIterator it;
        for (it = aboutData->credits().begin();
            it != aboutData->credits().end(); ++it)
        {
          creditsPage->addPerson( (*it).name(), (*it).emailAddress(),
                                    (*it).webAddress(), (*it).task() );
        }
    }

    const QValueList<KAboutTranslator> translatorList = aboutData->translators();

    if(translatorList.count() > 0) {
        KAboutContainer *translatorPage =
            addScrolledContainerPage( i18n("T&ranslation") );

        QValueList<KAboutTranslator>::ConstIterator it;
        for(it = translatorList.begin(); it != translatorList.end(); ++it) {
            translatorPage->addPerson((*it).name(), (*it).emailAddress(),
                                      0,0);
        }

        QLabel *label = new QLabel(KAboutData::aboutTranslationTeam(),
                                   translatorPage);
        label->adjustSize();
        label->setMinimumSize(label->sizeHint());
        translatorPage->addWidget(label);
    }

    if (!aboutData->license().isEmpty() ) {
        addLicensePage( i18n("&License Agreement"), aboutData->license() );
    }
}


//
// A class that can can monitor mouse movements on the image
//
// copied (and renamed) from kaboutdialog.cpp
// original: KImageTrackLabel

KRadioImageTrackLabel::KRadioImageTrackLabel( QWidget *_parent, const char *_name, WFlags f )
  : QLabel( _parent, _name, f )
{
  setText( i18n("Image missing"));
}

void KRadioImageTrackLabel::mousePressEvent( QMouseEvent *e )
{
  emit mouseTrack( MousePress, e );
}

void KRadioImageTrackLabel::mouseReleaseEvent( QMouseEvent *e )
{
  emit mouseTrack( MouseRelease, e );
}

void KRadioImageTrackLabel::mouseDoubleClickEvent( QMouseEvent *e )
{
  emit mouseTrack( MouseDoubleClick, e );
}

void KRadioImageTrackLabel::mouseMoveEvent ( QMouseEvent *e )
{
  emit mouseTrack( MouseDoubleClick, e );
}




#include "aboutwidget.moc"
