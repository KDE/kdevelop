//
// C++ Interface: annotationitem
//
// Description: 
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ANNOTATIONITEM_H
#define ANNOTATIONITEM_H

#include <klistview.h>
class QString;

/**
@author KDevelop Authors
*/
class annotationItem : public QListViewItem
{
public:
    annotationItem(QListView *parent,QString name, QString text);
    QString getName();
    QString getText();
    void setText(QString text);
    void setName(QString name);
    int getParent(){return m_itemParent;}

    ~annotationItem();
  private:
    annotationItem(QListViewItem *parent,QString name,QString text);
    QString m_itemName;
    QString m_itemText;
    int m_itemParent;
    
};

#endif
