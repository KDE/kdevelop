/***************************************************************************
                          ProjectOptionsDlg.h  -  description
                             -------------------
    begin                : Sat Jan 27 2001
    copyright            : (C) 2001 by Omid Givi
    email                : omid@givi.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PROJECTOPTIONSDLG_H
#define PROJECTOPTIONSDLG_H

#include "projectspace.h"
#include "kdevcomponent.h"
#include "KDevCompiler.h"
#include "ProjectOptionsDlgBase.h"
#include <qdialog.h>
#include <qmap.h>
#include "klocale.h"
#include "kdialogbase.h"

/**
  *@author Omid Givi
  */

class ProjectOptionsDlg: public ProjectOptionsDlgBase {
    Q_OBJECT

public:
    ProjectOptionsDlg(QWidget *parent=0, const char *name=0, ProjectSpace *ps=0);
    ~ProjectOptionsDlg();
protected slots:
	void slotTreeListItemSelected(QListViewItem *item );
	void slotButtonOkClicked();
	void slotButtonApplyClicked();

signals:
	void WidgetStarted(KDevCompiler*);
	void ButtonApplyClicked(KDevCompiler*);
	
protected:
  ProjectSpace *m_ps;
  QWidget* currentWidget;
  QWidget* nop;
  QWidget* dp;
  QWidget* gp;
  QWidget* ip;
  QMap<QListViewItem *, QWidget *> treeListToWidget;
};

#endif
