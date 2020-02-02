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

    virtual QString pluginClassName() const override { return QString::fromLatin1("QuickBar"); }

    virtual bool   connectI(Interface *i)    override;
    virtual bool   disconnectI(Interface *i) override;

    // IStationSelection

RECEIVERS:
    bool setStationSelection(const QStringList &sl) override;

ANSWERS:
    const QStringList & getStationSelection () const override { return m_stationIDs; }


    // PluginBase

public:
    virtual void   saveState   (KConfigGroup &) const override;
    virtual void   restoreState(const KConfigGroup &) override;
    virtual void   restoreState(const KConfigGroup &g, bool b) override { WidgetPluginBase::restoreState(g, b); }

    virtual ConfigPageInfo  createConfigurationPage() override;

    // IRadioClient

RECEIVERS:
    bool noticePowerChanged(bool on)                             override;
    bool noticeStationChanged (const RadioStation &, int idx)    override;
    bool noticeStationsChanged(const StationList &sl)            override;
    bool noticePresetFileChanged(const QUrl &/*f*/)              override { return false; }

    bool noticeRDSStateChanged      (bool  /*enabled*/)          override { return false; }
    bool noticeRDSRadioTextChanged  (const QString &/*s*/)       override { return false; }
    bool noticeRDSStationNameChanged(const QString &/*s*/)       override { return false; }

    bool noticeCurrentSoundStreamSourceIDChanged(SoundStreamID /*id*/) override { return false; }
    bool noticeCurrentSoundStreamSinkIDChanged  (SoundStreamID /*id*/) override { return false; }

    // button/station Management


protected slots:

    void    buttonToggled(const QString &stationID);

protected:

    void    uncheckAllOtherButtons(const QToolButton *b);
    void    activateCurrentButton();
    void    activateButton(const RadioStation &);

    void    autoSetCaption();


    void    dragEnterEvent(QDragEnterEvent* event) override;
    void    dropEvent(QDropEvent* event) override;

    // KDE/QT

public slots:

    virtual void    setGeometry (const QRect &r);
    virtual void    setGeometry (int x, int y, int w, int h);

public:
    virtual void     setVisible(bool v) override;

protected:
    void    rebuildGUI();
    void    showEvent(QShowEvent *) override;
    void    hideEvent(QHideEvent *) override;
    void    resizeEvent(QResizeEvent *) override;

    const QWidget *getWidget() const override { return this; }
          QWidget *getWidget()       override { return this; }

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
