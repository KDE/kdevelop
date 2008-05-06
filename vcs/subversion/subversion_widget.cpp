/**
	 Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>

	 This program is free software; you can redistribute it and/or
	 modify it under the terms of the GNU General Public
	 License as published by the Free Software Foundation; either
	 version 2 of the License, or (at your option) any later version.

	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	 General Public License for more details.

	 You should have received a copy of the GNU General Public License
	 along with this program; see the file COPYING.  If not, write to
	 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	 Boston, MA 02111-1307, USA.
	 */

#include <kparts/part.h>
#include <kdevcore.h>
#include <kdebug.h>
#include <klineedit.h>

#include "subversion_part.h"
#include "subversion_widget.h"
#include <ktextedit.h>
#include <klocale.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>

subversionWidget::subversionWidget( subversionPart *part, QWidget *parent, const char* name )
    : KTabWidget(parent)
{
	m_part = part;
    m_edit = new KTextEdit( this );
    m_edit->setReadOnly( TRUE );
    tab()->addTab( m_edit, i18n("Notification") );
    m_closeButton = new QPushButton( tab() );
    m_closeButton->setText( i18n("Close") );
    tab()->setCornerWidget(m_closeButton);
    connect( m_closeButton, SIGNAL(clicked()), this, SLOT(closeCurrentTab()) );
}

subversionWidget::~subversionWidget()
{}

void subversionWidget::append( QString notifications )
{
    if( !m_edit ){
        // should not happen
        m_edit = new KTextEdit(this);
    }
    m_edit->append( notifications );
    showPage( m_edit );
}

void subversionWidget::showLogResult( QValueList<SvnLogHolder> *holderList, QString reqUrl )
{
    SvnLogViewWidget *widget = new SvnLogViewWidget( m_part, this );
    widget->setLogResult( holderList );
	widget->setRequestedUrl( reqUrl );
    tab()->addTab( widget, i18n("Log History") );
    tab()->setTabEnabled( widget, true );
    tab()->showPage( widget );
}

void subversionWidget::showBlameResult( QValueList<SvnBlameHolder> *blamelist )
{
    SvnBlameWidget *widget = new SvnBlameWidget( this );
    widget->copyBlameData( blamelist );
    tab()->addTab( widget, i18n("Blame") );
    tab()->setTabEnabled( widget, true );
    tab()->showPage( widget );
}
void subversionWidget::closeCurrentTab()
{
    QWidget *current = tab()->currentPage();
    KTextEdit *edit = static_cast<KTextEdit*>(current);
    if( edit ){
        if( edit == m_edit ) // main notification output should not be deleted
            return;
    }
    tab()->removePage( current );
    delete current;
}

////////////////////////////////////////////////////////////////////////

SvnIntSortListItem::SvnIntSortListItem( QListView* parent )
	:QListViewItem(parent)
{}
SvnIntSortListItem::~SvnIntSortListItem()
{}

int SvnIntSortListItem::compare( QListViewItem *item, int col, bool ascending ) const
{
	
	unsigned int myVal = this->text(col).toUInt();
	unsigned int yourVal = item->text(col).toUInt();
	if( myVal < yourVal ) return -1;
	if( myVal > yourVal ) return 1;
	return 0;
}

SvnLogViewItem::SvnLogViewItem( QListView * parent )
    :SvnIntSortListItem( parent )
{
    m_pathList = "";
    m_message = "";
}
SvnLogViewItem ::~SvnLogViewItem ()
{}

////////////////////////////////////////////////////////////////////////

SvnProgressDlg::SvnProgressDlg( bool showNow )
	: KIO::DefaultProgress( showNow )
{
	setStopOnClose( true );
	setCaption( i18n("Subversion Job Progress") );
}

SvnProgressDlg::~SvnProgressDlg()
{}

void SvnProgressDlg::setSourceUrl( const QString &src )
{
	sourceEdit->setText( src );
}
void SvnProgressDlg::setDestUrl( const QString &dest )
{
	destEdit->setText( dest );
}

#include "subversion_widget.moc"
