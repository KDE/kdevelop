/***************************************************************************
   mdimainfrmcomponent.h  - a viewhandler (kdevcomponent) based on QextMDI
                             -------------------
    begin                : Thu Jul 27 2000
    copyright            : (C) 2000 by Falk Brettschneider
    email                : <Falk Brettschneider> falk@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MDIMAINFRMCOMPONENT_H_
#define _MDIMAINFRMCOMPONENT_H_

#include "kdevviewhandler.h"


class MdiFrame;

class MdiMainFrmComponent : public KDevViewHandler
{
  Q_OBJECT

public:
  MdiMainFrmComponent( QObject *parent=0, const char *name=0 );
  ~MdiMainFrmComponent();

  // This is currently called by MdiMainFrmComponentConfigWidget if the
  // changes are accepted. Maybe this could be done through
  // the component system (but maybe not ;-)
//  void configurationChanged();
  virtual bool event( QEvent *);

public slots:
  virtual void addView( QWidget*);
  virtual void removeView( QWidget*);

protected:
  virtual void setupGUI();
//  virtual void createConfigWidget(CustomizeDialog *parent);
  virtual void projectClosed();

private:
  MdiFrame *m_mainframe;
  friend class MdiFrame;

  QList<KAction>   m_viewList;
  QList<KAction>   m_viewsDockList;
};

#endif
