/***************************************************************************
                          kdevelop.h  -  description
                             -------------------
    begin                : Mit Jun 14 14:43:39 CEST 2000
    copyright            : (C) 2000 by The KDevelop Team
    email                : kdevelop-team@fara.cs.uni-potsdam.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVELOP_H
#define KDEVELOP_H

#include <kapp.h>
#include <kaction.h>
//#include <kparts/dockmainwindow.h>
#include "kdevcomponent.h"
#include <qextmdimainfrm.h>


class KDevelopCore;
class DCOPObject;
class KDCOPActionProxy;


class KDevelop : public QextMdiMainFrm //KParts::DockMainWindow
{
    Q_OBJECT

// types
public:
    typedef enum {
        EditorEnv       = 1
        ,DebugEnv       = 2
        ,TopLevelMode   = 16
        ,ChildframeMode = 32
    } DockSzenario;

public:
    /** construtor */
    KDevelop(QWidget* pParent = 0L, const char *name=0L , WFlags f = 0);
    /** destructor */
    ~KDevelop();
    /** Session management - writes the configuration */
    virtual void saveProperties(KConfig*);
    /** Session management - reads the configuration */
    virtual void readProperties(KConfig*);
  
    bool queryClose();
    bool queryExit();
public slots:
    /** Embed the widgets of components in the GUI. */
    void embedWidget(QWidget *w, KDevComponent::Role role, const QString &shortCaption, const QString &shortExplanation);
    /**
    * Undocks all view windows (unix-like)
    */
    virtual void switchToToplevelMode();
    /**
    * Docks all view windows (Windows-like)
    */
    virtual void switchToChildframeMode();

protected: // Protected methods
    /** fit the system menu button position to the menu position */
    virtual void resizeEvent( QResizeEvent * );

protected slots: // Protected slots
    /** reimplemented from KParts::MainWindow */
    //  void slotSetStatusBarText( const QString &text);

private:
    void initActions();
    /** initializes the help messages (whats this and
        statusbar help) on the KActions
        !!! isn't used anymore, exists for saving the help texts :-)) !!!
     */
    void initHelp();
    /**
     */
    void initQextMDI();
    /**
     */
    void restoreDockAndMdiSzenario( int dockSzenario);
    /**
     */
    void saveCurrentDockAndMdiSzenario();

    QWidget*                  m_pViewOnLeft;
    QWidget*                  m_pViewOnBottom;

    KDevelopCore*             m_pCore;

    QList<QextMdiChildView>   m_MDICoverList;

    int                       m_dockSzenario;

private slots:
    void slotOptionsEditToolbars();
};

#endif
