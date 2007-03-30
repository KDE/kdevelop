/*
 *  Copyright (C) 2007 Dukju Ahn (dukjuahn@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 */

#include "svn_blamewidget.h"
#include "subversion_widget.h"
#include <qmap.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstringlist.h>
#include <qfont.h>

#include <klocale.h>
#include <kmessagebox.h>

SvnBlameWidget::SvnBlameWidget( QWidget *parent, const char* name, bool modal, WFlags f )
	:QWidget( parent )
{
    m_layout = new QVBoxLayout( this, 1, 1 );
    m_layout->setMargin(1);
    
    m_listView = new QListView( this );
    outView()->setAllColumnsShowFocus( TRUE );
    outView()->addColumn( i18n("Line") );
    outView()->addColumn( i18n("Rev") );
    outView()->addColumn( i18n("Date") );
    outView()->addColumn( i18n("Author") );
    outView()->addColumn( i18n("Content") );
    
    m_layout->addWidget( m_listView );
}
SvnBlameWidget::~SvnBlameWidget()
{}

void SvnBlameWidget::copyBlameData( QValueList<SvnBlameHolder> *blamelist )
{
	m_blamelist = *blamelist;
}
	
void SvnBlameWidget::show()
{
	outView()->clear();
	outView()->setSortColumn(0);
	
	QFont f = outView()->font();
	f.setFixedPitch( true );
	outView()->setFont( f );

	QValueList<SvnBlameHolder>::Iterator it;
	
	for( it = m_blamelist.begin(); it != m_blamelist.end(); ++it ){
		
		SvnBlameHolder holder = *it;
		SvnIntSortListItem *item = new SvnIntSortListItem(outView());
		
		QString prettyDate = holder.date.left(16).replace(10, 1, ' ');
		
		item->setText(0, QString::number( holder.line+1 ) ); 
		item->setText(1, QString::number(holder.rev) );
		item->setText(2, prettyDate );
		item->setText(3, holder.author );
		item->setText(4, holder.content );
		
	}
	outView()->sort();
	QWidget::show();
}

QListView* SvnBlameWidget::outView()
{
    return m_listView;
}

/////////////////////////////////////////////////////////////

SvnBlameFileSelectDlg::SvnBlameFileSelectDlg( QWidget *parent )
    : QDialog( parent )
{
    m_selected = "";
    setCaption( i18n("Select one file to view annotation") );
    
    m_layout = new QGridLayout( this, 2, 2 );
    m_view = new QListView( this );
    m_view->addColumn( i18n("files") );
    m_okBtn = new QPushButton( i18n("OK"), this );
    m_cancelBtn = new QPushButton( i18n("Cancel"), this );
    m_layout->addMultiCellWidget( m_view, 0, 0, 0, 1 );
    m_layout->addWidget( m_okBtn, 1, 0 );
    m_layout->addWidget( m_cancelBtn, 1, 1 );
    
    connect( m_okBtn, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( m_cancelBtn, SIGNAL(clicked()), this, SLOT(reject()) );
}
SvnBlameFileSelectDlg::~SvnBlameFileSelectDlg()
{}

void SvnBlameFileSelectDlg::setCandidate( QStringList *list )
{
    for( QValueList<QString>::iterator it = list->begin(); it != list->end(); ++it ){
        QListViewItem *item = new QListViewItem( m_view, *it );
    }
}

QString SvnBlameFileSelectDlg::selected()
{
    return m_selected;
}

void SvnBlameFileSelectDlg::accept()
{
    while( true ){
        QListViewItem *item = m_view->currentItem();
        if( item ){
            m_selected = item->text(0);
            break;
        }
        else{
            KMessageBox::error( this, i18n("Select file from list to view annotation") );
        }
    }
    QDialog::accept();
}

#include "svn_blamewidget.moc"
