/***************************************************************************
                          quickbar.h  -  description
                             -------------------
    begin                : Mon Feb 11 2002
    copyright            : (C) 2002 by Martin Witte / Klas Kalass
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

#ifndef KRADIO_QUICKBAR_H
#define KRADIO_QUICKBAR_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "utils.h"

#include <qwidget.h>
#include "radio_interfaces.h"
#include "stationselection_interfaces.h"
#include "widgetplugins.h"

class ButtonFlowLayout;
class QButtonGroup;
class KConfig;
class QToolButton;

/**
  *@author Martin Witte / Klas Kalass
  */

class QuickBar : public QWidget,
                 public WidgetPluginBase,
                 public IRadioClient,
                 public IStationSelection
{
Q_OBJECT
public:
    QuickBar(QWidget * parent = 0, const char * name = 0);
    ~QuickBar();

    const QString &name() const { return PluginBase::name(); }
          QString &name()       { return PluginBase::name(); }

    virtual bool   connectI(Interface *i);
    virtual bool   disconnectI(Interface *i);

    // IStationSelection

RECEIVERS:
    bool setStationSelection(const QStringList &sl);

ANSWERS:
    const QStringList & getStationSelection () const { return m_stationIDs; }


    // PluginBase

public:
    virtual void   saveState (KConfig *) const;
    virtual void   restoreState (KConfig *);

    virtual ConfigPageInfo  createConfigurationPage();
    virtual AboutPageInfo   createAboutPage();

    // IRadioClient

RECEIVERS:
    bool noticePowerChanged(bool on);
    bool noticeStationChanged (const RadioStation &, int idx);
    bool noticeStationsChanged(const StationList &sl);
    bool noticePresetFileChanged(const QString &/*f*/)           { return false; }


    // button/station Management


protected slots:

    void    buttonClicked(int id);

protected:

    int     getButtonID(const RadioStation &rs) const;
    void    activateCurrentButton();
    void    activateButton(const RadioStation &);


    // KDE/QT

public slots:

    void    toggleShown() { WidgetPluginBase::pToggleShown(); }
    void    show();
    void    hide();
    void    setGeometry (const QRect &r);
    void    setGeometry (int x, int y, int w, int h);

protected:
    void    rebuildGUI();
    void    showEvent(QShowEvent *);
    void    hideEvent(QHideEvent *);
    void    resizeEvent(QResizeEvent *);

    const QWidget *getWidget() const { return this; }
          QWidget *getWidget()       { return this; }

protected :

    ButtonFlowLayout *m_layout;
    QButtonGroup     *m_buttonGroup;

    QPtrList<QToolButton> m_buttons;

    // config
    bool              m_showShortName;
    QStringList       m_stationIDs;

    bool              m_ignoreNoticeActivation;
};
#endif
