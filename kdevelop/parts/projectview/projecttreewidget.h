/***************************************************************************
 *   Copyright (C) 2000 by Sandy Meier                                     *
 *   smeier@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PROJECTTREEWIDGET_H_
#define _PROJECTTREEWIDGET_H_

#include <qlistview.h>

class ProjectView;
class ProjectSpace;

class ProjectTreeWidget : public QListView
{
    Q_OBJECT
    
public:
    ProjectTreeWidget(ProjectView* pPart);
    ~ProjectTreeWidget();
    void setProjectSpace(ProjectSpace* pProjectSpace);
 protected:
    
    ProjectSpace* m_pProjectSpace;
};

#endif
