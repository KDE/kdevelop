/***************************************************************************                                *   Copyright (C) 2000 by Sandy Meier                                     *
 *   smeier@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kpopupmenu.h>
#include "projecttreewidget.h"
#include "projectspace.h"


ProjectTreeWidget::ProjectTreeWidget(ProjectView *part)
  : QListView(0, "project tree widget"){
  m_pProjectSpace = 0;
 
}

ProjectTreeWidget::~ProjectTreeWidget()
{}

void ProjectTreeWidget::setProjectSpace(ProjectSpace* pProjectSpace){
  m_pProjectSpace =pProjectSpace;
}


