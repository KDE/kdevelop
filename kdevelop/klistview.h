/***************************************************************************
                          klistview.h  -  Smart list view
                             -------------------
    begin                : Sat Jun 26 1999
    copyright            : (C) 1999 by Bernd Gehrmann
    email                : bernd@physik.hu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Optionally, you can choose to distribute this file under the          *
 *   conditions of the GNU Library General Public License.                 *
 *                                                                         *
 ***************************************************************************/


#ifndef _KLISTVIEW_H_
#define _KLISTVIEW_H_

#include <qlistview.h>


/**
 * A value-added variant of QListView
 *
 * In addition to QListView's features, this class makes
 * the last inserted child available as lastChild().
 */
class KListView : public QListView
{
    Q_OBJECT
public:
    KListView( QWidget *parent=0, const char *name=0 );
    ~KListView();
    virtual void insertItem(QListViewItem *item);
    virtual void clear();
    QListViewItem *lastChild() const
	{ return last; }

 protected:
    virtual void contentsMousePressEvent(QMouseEvent * event);
    
private:
    QListViewItem *last;
};


/**
 * A less braindead variant of QListViewItem
 *
 * In contrast to QListViewItem, KListViewItem always inserts
 * itself at the end of its parent's children list. So it
 * becomes possible to simply construct one child after the
 * other, without the strange effect of getting them displayed
 * in the reverse order.
 *
 * Moreover, it maintains a string that is not displayed
 * and which is available by ident().
 */
class KListViewItem : public QListViewItem
{
public: 
    KListViewItem( KListView *parent, const char *text, const char *id );
    KListViewItem( KListViewItem *parent, const char *text, const char *id );
    virtual ~KListViewItem();
    virtual void insertItem(QListViewItem *item);
    QListViewItem *lastChild() const
	{ return last; }
    QString ident() const
	{ return idnt; }
    
private:
    QListViewItem *last;
    QString idnt;
};


class KCheckListItem : public QCheckListItem
{
public: 
    KCheckListItem( QListView *parent, const char *text, const char *id );
    QString ident() const
	{ return idnt; }
    
private:
    QString idnt;
};
#endif
