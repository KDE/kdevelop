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
#include <qdialog.h>
#include <qmap.h>
#include "klocale.h"
#include "kdialogbase.h"

/**
  *@author Omid Givi
  */

class KDevCompiler;

class ProjectOptionsDlg: public KDialogBase {
    Q_OBJECT

public:
    ProjectOptionsDlg(QWidget *parent=0, const char *name=0, ProjectSpace *ps=0);
    ~ProjectOptionsDlg();
protected slots:
	void slotOk();
	void slotApply();

signals:
	void WidgetStarted();
	void ButtonApplyClicked();
	
protected:
  ProjectSpace *m_ps;
};

#endif
