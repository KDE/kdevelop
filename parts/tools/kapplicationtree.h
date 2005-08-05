/* This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef __KDevApplicationTree_h__
#define __KDevApplicationTree_h__

#include <kurl.h>
#include <klistview.h>

class KURLRequester;

class QWidget;
class QCheckBox;
class QPushButton;
class QLabel;
class QStringList;


/* ------------------------------------------------------------------------- */

/**
 * @internal
 */
class KDevAppTreeListItem : public QListViewItem
{
    bool parsed;
    bool directory;
    QString path;
    QString exec;
    QString dEntry;

protected:
    QString key(int column, bool ascending) const;

    void init(const QPixmap& pixmap, bool parse, bool dir, const QString& _path, const QString& _exec, const QString& _dEntry);

public:
    KDevAppTreeListItem( KListView* parent, const QString & name, const QPixmap& pixmap,
                      bool parse, bool dir, const QString& p, const QString& c, const QString& dE );
    KDevAppTreeListItem( QListViewItem* parent, const QString & name, const QPixmap& pixmap,
                      bool parse, bool dir, const QString& p, const QString& c, const QString& dE );
    bool isDirectory();

    QString desktopEntryPath() const { return dEntry; }
    QString executable() const { return exec; }

protected:
    virtual void activate();
    virtual void setOpen( bool o );

    friend class KDevApplicationTree;
};

/* ------------------------------------------------------------------------- */

/**
 * @internal
 */
class KDevApplicationTree : public KListView
{
    Q_OBJECT
public:
    KDevApplicationTree( QWidget *parent, const char* name = 0 );

    /**
     * Add a group of .desktop/.kdelnk entries
     */
    void addDesktopGroup( QString relPath, KDevAppTreeListItem *item = 0 );

    bool isDirSel();

protected:
    void resizeEvent( QResizeEvent *_ev );
    KDevAppTreeListItem* currentitem;

public slots:
    void slotItemHighlighted(QListViewItem* i);
    void slotSelectionChanged(QListViewItem* i);

signals:
    void selected( const QString& _name, const QString& _exec );
    void highlighted( const QString& _name, const  QString& _exec );
};

/* ------------------------------------------------------------------------- */

#endif
