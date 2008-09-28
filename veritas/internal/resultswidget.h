/*
 * This file is part of KDevelop
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef VERITAS_RESULTSWIDGET_H
#define VERITAS_RESULTSWIDGET_H

#include <QTreeView>
#include "../veritasexport.h"

namespace Veritas
{

class VERITAS_EXPORT ResultsWidget : public QTreeView
{
public:
    ResultsWidget(QWidget* parent=0);
    virtual ~ResultsWidget();

    /*! WARNING Since recently, Qt asserts if no model with the right 
     *  number of of columns (4 in this case) has been set.
     *  So _first_ initialize a resultsmodel on this widget's QTreeView 
     *  or crash. */
    void setResizeMode();
    QTreeView* tree();
};

}

#endif // VERITAS_RESULTSWIDGET_H
