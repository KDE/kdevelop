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
#include <kparts/dockmainwindow.h>
#include "kdevcomponent.h"


class KDevelopCore;


class KDevelop : public KParts::DockMainWindow
{
 Q_OBJECT
public:
  /** construtor */
  KDevelop(const char *name=0);
  /** destructor */
  ~KDevelop();

public slots:
  /** Embed the widgets of components in the GUI. */
  void embedWidget(QWidget *w, KDevComponent::Role role, const QString &shortCaption, const QString &shortExplanation);
  /** stacking document views as tabbed KDockWidgets is default behaviour if no view handler library was found.
   *  Usually connected in KDevelopCore::loadInitialComponents. */
  void stackView( QWidget*);

protected slots: // Protected slots
  /** reimplemented from KParts::MainWindow */
//  void slotSetStatusBarText( const QString &text);

signals:
  void addView( QWidget*);

private:

 void initActions(); 
  /** initializes the help messages (whats this and
      statusbar help) on the KActions 
      !!! isn't used anymore, exists for saving the help texts :-)) !!!
  */ 
  void initHelp();

 
  //
  // the covering dockwidgets for all views
  //
  /** The initial dock cover for document views */
  KDockWidget* m_dockbaseAreaOfDocumentViews;

  // TODO: Workaround for bug in dock window stuff when using protected members
  // DockL and DockB
  KDockWidget* m_dockOnLeft;
  KDockWidget* m_dockOnBottom;

  KDevelopCore* m_pCore;

private slots:
 void slotOptionsEditToolbars();
};

#endif
