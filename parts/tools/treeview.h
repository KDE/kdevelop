/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __treeview_h__
#define __treeview_h__


#include <qstring.h>


#include <klistview.h>
#include <kservicegroup.h>


class QPopupMenu;


class TreeItem : public QListViewItem
  {

public:
    TreeItem(QListViewItem *parent, KService* service = 0);
    TreeItem(QListViewItem *parent, QListViewItem *after, KService* service = 0);
    TreeItem(QListView *parent, KService* service = 0);
    TreeItem(QListView *parent, QListViewItem* after, KService* service = 0);
    
    void init( KService* service );

    QString file() const { return _file; }
    QString exec() const { return _exec; }
    QString caption() const { return _caption; }
    
private:
    QString _file, _exec, _caption;
  };


class TreeView : public KListView
  {
    Q_OBJECT

public:
    TreeView(QWidget *parent = 0, const char *name = 0);

signals:
    void entrySelected(const QString&);


protected:
    void fill();
    void fillBranch(const QString& relPath, TreeItem* parent);

private:
    QPopupMenu *_rmb;

    void decorateItem(TreeItem *item, KServiceGroup::Ptr g);
    void decorateItem(TreeItem *item, KService::Ptr s);

  };

#endif
