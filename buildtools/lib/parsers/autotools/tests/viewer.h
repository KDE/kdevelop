/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef VIEWER_H
#define VIEWER_H

#include <q3valuestack.h>

#include "viewerbase.h"

namespace AutoTools { class ProjectAST; }

class Q3ListViewItem;

class Viewer: public ViewerBase {
Q_OBJECT
public:
    Viewer(QWidget *parent = 0, const char *name = 0);
    void processAST(AutoTools::ProjectAST *projectAST, Q3ListViewItem *globAfter = 0);
public slots:
    virtual void tabWidget2_selected(const QString&);
    virtual void files_currentChanged(Q3ListBoxItem*);
    virtual void choose_clicked();
    virtual void addAll_clicked();
private:
    AutoTools::ProjectAST *projectAST;
    Q3ValueStack<Q3ListViewItem *> parentProject;
};

#endif
