/***************************************************************************
                          kdevlistview.h  -  Smart list view
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


#ifndef _KDEVLISTVIEW_H_
#define _KDEVLISTVIEW_H_

#include <qlistview.h>


/**
 * A value-added variant of QListView
 *
 * In addition to QListView's features, this class makes
 * the last inserted child available as lastChild().
 */
class KDevListView : public QListView
{
    Q_OBJECT
public:
    KDevListView( QWidget *parent=0, const char *name=0 );
    virtual ~KDevListView();
    virtual void insertItem(QListViewItem *item);
    QListViewItem *lastChild() const;
 protected:
    virtual void mousePressEvent(QMouseEvent * event);
};


/**
 * A less braindead variant of QListViewItem
 *
 * In contrast to QListViewItem, KDevListViewItem always inserts
 * itself at the end of its parent's children list. So it
 * becomes possible to simply construct one child after the
 * other, without the strange effect of getting them displayed
 * in the reverse order.
 *
 * Moreover, it maintains a string that is not displayed
 * and which is available by ident().
 */
class KDevListViewItem : public QListViewItem
{
public: 
    KDevListViewItem( KDevListView *parent, const char *text, const char *id );
    KDevListViewItem( KDevListViewItem *parent, const char *text, const char *id );
    virtual ~KDevListViewItem();
    virtual void insertItem(QListViewItem *item);
    QListViewItem *lastChild() const;
    QString ident() const
	{ return idnt; }
    void setIdent(const QString &id)
	{ idnt=id; }

private:
    QString idnt;
};

#endif
