/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef ACTIONLISTVIEW_H
#define ACTIONLISTVIEW_H

#include <qlistview.h>
#include "actiondnd.h"

class ActionItem : public QListViewItem
{
public:
    ActionItem( QListView *lv, bool group )
	: QListViewItem( lv ),
	  a( group ? 0 : new QDesignerAction( 0 ) ),
	  g( group ? new QDesignerActionGroup( 0 ) : 0 ) { setDragEnabled( TRUE ); }
    ActionItem( QListView *lv, QAction *ac );
    ActionItem( QListViewItem *i, QAction *ac );
    ActionItem( ActionItem *parent, bool group = FALSE )
	: QListViewItem( parent ),
	  a( group ? 0 : new QDesignerAction( parent->actionGroup() ) ),
	  g( group ? new QDesignerActionGroup( parent->actionGroup() ) : 0 ) { setDragEnabled( TRUE ); moveToEnd(); }

    QDesignerAction *action() const { return a; }
    QDesignerActionGroup *actionGroup() const { return g; }

private:
    void moveToEnd();

private:
    QDesignerAction *a;
    QDesignerActionGroup *g;

};

class ActionListView : public QListView
{
    Q_OBJECT

public:
    ActionListView( QWidget *parent = 0, const char *name = 0 );

protected:
    QDragObject *dragObject();

private slots:
    void rmbMenu( QListViewItem *i, const QPoint &p );

signals:
    void insertAction();
    void insertActionGroup();
    void insertDropDownActionGroup();
    void deleteAction();
    void connectAction();

};

#endif
