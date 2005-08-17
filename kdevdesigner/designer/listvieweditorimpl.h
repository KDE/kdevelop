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

#ifndef LISTVIEWEDITORIMPL_H
#define LISTVIEWEDITORIMPL_H

#include "listvieweditor.h"

#include <qmap.h>
#include <qpixmap.h>
#include <q3valuelist.h>

class FormWindow;

class ListViewEditor : public ListViewEditorBase
{
    Q_OBJECT

public:
    ListViewEditor( QWidget *parent, Q3ListView *lv, FormWindow *fw );

signals:
    void itemRenamed(const QString &);

protected slots:
    void applyClicked();
    void columnClickable(bool);
    void columnDownClicked();
    void columnPixmapChosen();
    void columnPixmapDeleted();
    void columnResizable(bool);
    void columnTextChanged(const QString &);
    void columnUpClicked();
    void currentColumnChanged(Q3ListBoxItem*);
    void currentItemChanged(Q3ListViewItem*);
    void deleteColumnClicked();
    void itemColChanged(int);
    void itemDeleteClicked();
    void itemDownClicked();
    void itemNewClicked();
    void itemNewSubClicked();
    void itemPixmapChoosen();
    void itemPixmapDeleted();
    void itemTextChanged(const QString &);
    void itemUpClicked();
    void itemLeftClicked();
    void itemRightClicked();
    void newColumnClicked();
    void okClicked();
    void initTabPage(const QString &page);
    void emitItemRenamed(Q3ListViewItem*, int, const QString&); // signal relay

private:
    struct Column
    {
	Q3ListBoxItem *item;
	QString text;
	QPixmap pixmap;
	bool clickable, resizable;
	Q_DUMMY_COMPARISON_OPERATOR( Column )
    };

private:
    void setupColumns();
    void setupItems();
    Column *findColumn( Q3ListBoxItem *i );
    void transferItems( Q3ListView *from, Q3ListView *to );
    void displayItem( Q3ListViewItem *i, int col );

private:
    Q3ListView *listview;
    Q3ValueList<Column> columns;
    int numColumns;
    FormWindow *formwindow;

};


#endif
