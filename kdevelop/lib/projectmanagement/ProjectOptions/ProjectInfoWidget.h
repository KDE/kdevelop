/***************************************************************************
                          ProjectInfoWidget.h  -  description
                             -------------------
    begin                : Mon Mar 5 2001
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

#ifndef PROJECTINFOWIDGET_H
#define PROJECTINFOWIDGET_H

#include <qwidget.h>
#include <ProjectInfoWidgetBase.h>

/**
  *@author Omid Givi
  */

class ProjectSpace;

class ProjectInfoWidget : public ProjectInfoWidgetBase  {
   Q_OBJECT
public: 
	ProjectInfoWidget(QWidget *parent=0, const char *name=0, ProjectSpace* ps=0, QWidget* pdlg=0);
	~ProjectInfoWidget();

public slots:
  void slotButtonApplyClicked();

private:
  ProjectSpace* m_pProjectSpace;
};

#endif
