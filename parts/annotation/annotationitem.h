/***************************************************************************
 *   Copyright (C) 2005 by Mathieu Chouinard                               *
 *   mchoui@e-c.qc.ca                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#ifndef ANNOTATIONITEM_H
#define ANNOTATIONITEM_H

#include <klistview.h>
class QString;

/**
@author KDevelop Authors
*/
class annotationItem : public Q3ListViewItem
{
public:
    annotationItem(Q3ListView *parent,QString name, QString text);
    QString getName();
    QString getText();
    void setText(QString text);
    void setName(QString name);
    int getParent(){return m_itemParent;}

    ~annotationItem();
  private:
    annotationItem(Q3ListViewItem *parent,QString name,QString text);
    QString m_itemName;
    QString m_itemText;
    int m_itemParent;
    
};

#endif
