/***************************************************************************
 *   Copyright (C) 2003 by Marcel Turino                              *
 *   M.Turino@gmx.de                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CHOOSEDLG_H_
#define _CHOOSEDLG_H_

#include <qdialog.h>
#include <qlineedit.h>
#include <qptrlist.h>
#include <klistview.h>
#include <qlabel.h>

#include "doctreeviewpart.h"

class IndexTreeData;

class ChooseDlg : public QDialog
{
    Q_OBJECT

public:
    ChooseDlg( QWidget *parent=0, const char *name=0, DocTreeViewPart *part = 0 );
    ~ChooseDlg();

    void setList(const QPtrList<IndexTreeData> *list);
    
private slots:
    void slotItemExecuted(QListViewItem *item);

private:
    KListView *lv;
    DocTreeViewPart *m_part;
};

#endif
