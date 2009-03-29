/***************************************************************************
                          aboutwidget.h  -  description
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

#ifndef KRADIO_ABOUT_WIDGET_H
#define KRADIO_ABOUT_WIDGET_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


//#include <QtGui/QWidget>
#include <QtGui/QLabel>

/* Unfortunately KDE doesn't provide the class KAboutContainerBase
   to public programming, so we have to copy most of that code into
   an own class :(
*/

class QFrame;
class KAboutContainer;
class QTabWidget;
class QVBoxLayout;
class KAboutData;

#include <QtGui/QLabel>

// copied (and renamed) from kaboutdialog_private.h
// original: KImageTrackLabel

class KRadioImageTrackLabel : public QLabel
{
  Q_OBJECT

  public:
    enum MouseMode
    {
      MousePress = 1,
      MouseRelease,
      MouseDoubleClick,
      MouseMove
    };

  public:
    KRadioImageTrackLabel( QWidget * parent, const char * name=0);

  signals:
    void mouseTrack( int mode, const QMouseEvent *e );

  protected:
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void mouseReleaseEvent( QMouseEvent *e );
    virtual void mouseDoubleClickEvent( QMouseEvent *e );
    virtual void mouseMoveEvent ( QMouseEvent *e );
};

// copied (and renamed) from kaboutdialog_private.h
// original: KAboutContainerBase

class KRadioAboutWidget : public QWidget
{
  Q_OBJECT

  public:
    enum LayoutType
    {
      AbtPlain         = 0x0001,
      AbtTabbed        = 0x0002,
      AbtTitle         = 0x0004,
      AbtImageLeft     = 0x0008,
      AbtImageRight    = 0x0010,
      AbtImageOnly     = 0x0020,
      AbtProduct       = 0x0040,
      AbtKDEStandard   = AbtTabbed|AbtTitle|AbtImageLeft,
      AbtAppStandard   = AbtTabbed|AbtTitle|AbtProduct,
      AbtImageAndTitle = AbtPlain|AbtTitle|AbtImageOnly
    };

  public:
    KRadioAboutWidget(const KAboutData &abtData, int layoutType, QWidget *parent = 0, char *name = 0);
    virtual void show( void );
    virtual QSize sizeHint( void ) const;

    void setAboutData(const KAboutData &abtData);

    void setTitle( const QString &title );
    void setImage( const QString &fileName );
    void setImageBackgroundColor( const QColor &color );
    void setImageFrame( bool state );
    void setProduct( const QString &appName, const QString &version,
             const QString &author, const QString &year );

    QFrame *addTextPage( const QString &title, const QString &text,
             bool richText=false, int numLines=10 );
    QFrame *addLicensePage( const QString &title, const QString &text,
             int numLines=10 );
    KAboutContainer *addContainerPage( const QString &title,
      int childAlignment = AlignCenter, int innerAlignment = AlignCenter );
    KAboutContainer *addScrolledContainerPage( const QString &title,
      int childAlignment = AlignCenter, int innerAlignment = AlignCenter );

    QFrame *addEmptyPage( const QString &title );

    KAboutContainer *addContainer( int childAlignment, int innerAlignment );

  public slots:
    virtual void slotMouseTrack( int mode, const QMouseEvent *e );
    virtual void slotUrlClick( const QString &url );
    virtual void slotMailClick( const QString &name, const QString &address );

  protected:
    virtual void fontChange( const QFont &oldFont );

  signals:
    void mouseTrack( int mode, const QMouseEvent *e );
    void urlClick( const QString &url );
    void mailClick( const QString &name, const QString &address );

  private:
    QMemArray<QWidget*>  mContainerList;

    QVBoxLayout *mTopLayout;
    KRadioImageTrackLabel *mImageLabel;
    QLabel  *mTitleLabel;
    QLabel  *mIconLabel;
    QLabel  *mVersionLabel;
    QLabel  *mAuthorLabel;
    QFrame  *mImageFrame;
    QTabWidget *mPageTab;
    QFrame  *mPlainSpace;
};


#endif
