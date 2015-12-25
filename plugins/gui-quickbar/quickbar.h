/***************************************************************************
                          quickbar.h  -  description
                             -------------------
    begin                : Mon Feb 11 2002
    copyright            : (C) 2002 by Martin Witte / Klas Kalass
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

#ifndef KRADIO_QUICKBAR_H
#define KRADIO_QUICKBAR_H

#include <QWidget>
#include <QSignalMapper>


#include "radio_interfaces.h"
#include "widgetpluginbase.h"
#include "stationselection_interfaces.h"

class ButtonFlowLayout4;
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
    QuickBar(const QString &instanceID, const QString &name);
    ~QuickBar();

    virtual QString pluginClassName() const { return "QuickBar"; }

//     const QString &name() const { return PluginBase::name(); }
//           QString &name()       { return PluginBase::name(); }

    virtual bool   connectI(Interface *i);
    virtual bool   disconnectI(Interface *i);

    // IStationSelection

RECEIVERS:
    bool setStationSelection(const QStringList &sl);

ANSWERS:
    const QStringList & getStationSelection () const { return m_stationIDs; }


    // PluginBase

public:
    virtual void   saveState   (KConfigGroup &) const;
    virtual void   restoreState(const KConfigGroup &);
    virtual void   restoreState(const KConfigGroup &g, bool b) { WidgetPluginBase::restoreState(g, b); }

    virtual ConfigPageInfo  createConfigurationPage();

    // IRadioClient

RECEIVERS:
    bool noticePowerChanged(bool on);
    bool noticeStationChanged (const RadioStation &, int idx);
    bool noticeStationsChanged(const StationList &sl);
    bool noticePresetFileChanged(const QString &/*f*/)           { return false; }

    bool noticeRDSStateChanged      (bool  /*enabled*/)          { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)       { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/)       { return false; }

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/) { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/) { return false; }

    // button/station Management


protected slots:

    void    buttonToggled(const QString &stationID);

protected:

    void    uncheckAllOtherButtons(const QToolButton *b);
    void    activateCurrentButton();
    void    activateButton(const RadioStation &);

    void    autoSetCaption();


    void    dragEnterEvent(QDragEnterEvent* event);
    void    dropEvent(QDropEvent* event);

    // KDE/QT

public slots:

    virtual void    toggleShown() { WidgetPluginBase::pToggleShown(); }
    virtual void    setGeometry (const QRect &r);
    virtual void    setGeometry (int x, int y, int w, int h);

public:
    virtual void     setVisible(bool v);

protected:
    void    rebuildGUI();
    void    showEvent(QShowEvent *);
    void    hideEvent(QHideEvent *);
    void    resizeEvent(QResizeEvent *);

    const QWidget *getWidget() const { return this; }
          QWidget *getWidget()       { return this; }

protected :

    ButtonFlowLayout4  *m_layout;

    QSignalMapper       m_mapper;
    QList<QToolButton*> m_buttons;

    // config
    bool                m_showShortName;
    QStringList         m_stationIDs;

    bool                m_ignoreNoticeActivation;
};
#endif
