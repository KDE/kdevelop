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

#include "choosedlg.h"

#include <qcombobox.h>
#include <qfile.h>
#include <qdir.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <kapplication.h>
#include <kbuttonbox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <qtimer.h>

#include "kdevpartcontroller.h"
#include "kdevmainwindow.h"
#include "doctreeviewwidget.h"

ChooseDlg::ChooseDlg(QWidget *parent, const char *name, DocTreeViewPart *part)
    : QDialog(parent, name, true), m_part(part)
{    
    setCaption(i18n("Documentation"));

    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());

    layout->addWidget(new QLabel(i18n("Choose your help subtopic:"), this));
    lv = new KListView(this, "topics list");
    lv->setMinimumWidth(600);
	lv->setAllColumnsShowFocus( true );
    lv->addColumn(i18n("Topic"));
    lv->addColumn(i18n("File Name"));
    layout->addWidget(lv);
        
    connect( lv, SIGNAL ( executed(QListViewItem *) ), this, SLOT ( slotItemExecuted(QListViewItem *) ) );
    connect( lv, SIGNAL ( returnPressed(QListViewItem *) ), this, SLOT ( slotItemExecuted(QListViewItem *) ) );
}

   
    
ChooseDlg::~ChooseDlg()
{}

void ChooseDlg::setList(const QPtrList<IndexTreeData> *list)
{
    QListViewItem *lvi;
    IndexTreeData *itd;
    QPtrListIterator<IndexTreeData> ptrListIterator( *list );
    
    while( ptrListIterator.current() )
    {
        itd = static_cast<IndexTreeData *>(ptrListIterator.current());
        lvi = new QListViewItem(lv, itd->parent(), itd->fileName());
        
        ++ptrListIterator;
    } 
}

void ChooseDlg::slotItemExecuted(QListViewItem *item)
{
    if(item == 0) return;

    m_part->partController()->showDocument(KURL( item->text(1) ));
    m_part->mainWindow()->lowerView(this);
    
    close();
}

#include "choosedlg.moc"
