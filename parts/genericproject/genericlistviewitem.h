/***************************************************************************
 *   Copyright (C) 2003 Roberto Raggi                                      *
 *   roberto@kdevelop.org                                                  *
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef GENERICLISTVIEWITEM_H
#define GENERICLISTVIEWITEM_H

#include <klistview.h>

class QSimpleRichText;
class BuildFileItem;
class BuildTargetItem;
class BuildGroupItem;
class BuildBaseItem;

class GenericListViewItem : public KListViewItem
{
public:
    GenericListViewItem( bool bold, QListView* parent, const QString& text );
    GenericListViewItem( bool bold, QListViewItem* parent, const QString& text );
    virtual ~GenericListViewItem();
    
    bool isBold() const { return m_bold; }
    void setBold( bool b );
    
    virtual void paintCell( QPainter* p, const QColorGroup& cg, int column, int width, int alignment );

    virtual BuildBaseItem* buildItem() const;
protected:
    bool m_bold;

    BuildBaseItem *m_buildItem;
};

class GenericFileListViewItem: public GenericListViewItem
{
public:
    GenericFileListViewItem( QListView* parent, BuildFileItem* fileItem );
    GenericFileListViewItem( QListViewItem* parent, BuildFileItem* fileItem );
    BuildFileItem* fileItem() const;

    virtual void setup();
};

class GenericTargetListViewItem: public GenericListViewItem
{
public:
    GenericTargetListViewItem( QListView* parent, BuildTargetItem* targetItem );
    GenericTargetListViewItem( QListViewItem* parent, BuildTargetItem* targetItem );
    BuildTargetItem* targetItem() const;

    virtual void setup();
};

class GenericGroupListViewItem: public GenericListViewItem
{
public:
    GenericGroupListViewItem( QListView* parent, BuildGroupItem* groupItem );
    GenericGroupListViewItem( QListViewItem* parent, BuildGroupItem* groupItem );

    BuildGroupItem* groupItem() const;
    void init();
    virtual void setup();
};


#endif
