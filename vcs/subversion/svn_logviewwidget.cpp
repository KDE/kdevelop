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
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#include "svn_logviewwidget.h"
#include "svn_blamewidget.h"
#include "subversion_core.h"
#include "subversion_global.h"
#include <kdevproject.h>
#include <ktextedit.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <klocale.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <knuminput.h>
#include <qcheckbox.h>

#include <qsplitter.h>
#include <qheader.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qstringlist.h>

SvnLogViewWidget::SvnLogViewWidget(subversionPart *part, QWidget *parent)
	:QWidget(parent), m_part(part)
{
    m_layout = new QGridLayout( this, 1, 1, 11, 6, "SvnLogViewWidgetBaseLayout");

    splitter1 = new QSplitter( this, "splitter1" );
    splitter1->setOrientation( QSplitter::Horizontal );
    splitter1->setMargin(1);

    listView1 = new QListView( splitter1, "listView1" );
    listView1->addColumn( i18n( "Rev" ) );
    listView1->addColumn( i18n( "Date" ) );
    listView1->addColumn( i18n( "Author" ) );
    listView1->addColumn( i18n( "Comment" ) );
    listView1->resize( QSize(1, 1).expandedTo(minimumSizeHint()) );
    QFont listView1_font(  listView1->font() );
    listView1_font.setPointSize( 9 );
    listView1->setFont( listView1_font );
    listView1->setAllColumnsShowFocus( TRUE );
    listView1->setShowSortIndicator( TRUE );

    textEdit1 = new KTextEdit( splitter1, "textEdit1" );
    textEdit1->resize( QSize(1, 1).expandedTo(minimumSizeHint()) );
    QFont textEdit1_font(  textEdit1->font() );
    textEdit1_font.setPointSize( 9 );
    textEdit1->setFont( textEdit1_font );
    textEdit1->setFocusPolicy( QTextEdit::WheelFocus );
    textEdit1->setReadOnly( TRUE );

    m_layout->addWidget( splitter1, 0, 0 );
    m_layout->setMargin(1);

    resize( QSize(692, 343).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
    
	connect( listView1, SIGNAL(clicked( QListViewItem *)), this, SLOT(slotClicked(QListViewItem*)) );
    connect( listView1, SIGNAL(contextMenuRequested( QListViewItem*, const QPoint&, int )),
             this, SLOT(contextMenuRequested(QListViewItem*, const QPoint&, int)) );
}
SvnLogViewWidget::~SvnLogViewWidget()
{
}

void SvnLogViewWidget::setLogResult( QValueList<SvnLogHolder> *loglist )
{
	this->listView1->clear();
	this->textEdit1->clear();
	this->listView1->setSorting( 1, false );
	
	for( QValueList<SvnLogHolder>::Iterator it=loglist->begin(); it!=loglist->end(); ++it ){
		
		SvnLogHolder holder = *it;
		SvnLogViewItem *item = new SvnLogViewItem(this->listView1);
		
		QString prettyDate = holder.date.left(16).replace(10, 1, ' ');
		
		item->setText(0, holder.rev );
		item->setText(1, prettyDate );
		item->setText(2, holder.author );
		item->setText(3, holder.logMsg.simplifyWhiteSpace() );
		
		item->m_pathList = holder.pathList;
		item->m_message = holder.logMsg;
	}
// 	this->listView1->show();
}

void SvnLogViewWidget::setRequestedUrl( QString reqUrl )
{
	m_reqUrl = reqUrl;
}

void SvnLogViewWidget::slotClicked( QListViewItem *oneItem )
{
	if( !oneItem ) return;
	SvnLogViewItem *item = dynamic_cast<SvnLogViewItem*>( oneItem );
    if( !item ) return;
	textEdit1->clear();
	textEdit1->append( item->m_pathList );
	textEdit1->append( "\n\n" );
	textEdit1->append( item->m_message + "\n" );
}
void SvnLogViewWidget::contextMenuRequested( QListViewItem *item, const QPoint & pos, int col )
{
    if( !item || col == -1 )
        return;
    m_ctxLogItem = dynamic_cast<SvnLogViewItem*>(item);
    if( !m_ctxLogItem )
        return;
    QPopupMenu *menu = new QPopupMenu(this);
    menu->insertItem( i18n("Blame this revision"), this, SLOT(blameThis()) );
    menu->insertItem( i18n("Difference to previous revision"), this, SLOT(diffToPrevious()) );
    menu->exec( pos );
}
void SvnLogViewWidget::blameThis()
{
	if( !m_ctxLogItem ){
		KMessageBox::error( this, i18n("No revision was clicked"), i18n("error") );
		return;
	}
	// note that blame is done on single file.
	QStringList modifies = QStringList::split( "\n", m_ctxLogItem->m_pathList, false );
	QString selectedPath;
	if( modifies.count() > 1 ){
		SvnBlameFileSelectDlg dlg(this);
		dlg.setCandidate( &modifies );
		if( dlg.exec() == QDialog::Accepted ){
			selectedPath = dlg.selected();
		} else{
			return;
		}
		
	} else if( modifies.count() == 1 ){
		selectedPath = *( modifies.at(0) );
	} else {
		return;
	}
	
	QString relPath = selectedPath.section( '/', 1 );
	
	QValueList< SvnGlobal::SvnInfoHolder > holderList = m_part->m_prjInfoMap.values();
	SvnGlobal::SvnInfoHolder holder;
	if( holderList.count() > 0 ){
		// get full Url
		holder = holderList.first();
		QString absPath =  holder.reposRootUrl.url(-1) + '/' + relPath;
		kdDebug(9036) << " Blame requested on path " << absPath << endl;
		// get revision
		int revEnd = m_ctxLogItem->text(0).toInt();
		// final request
		m_part->svncore()->blame( KURL(absPath), SvnGlobal::dont_touch, 0, "", revEnd, "" );
	}
	else{
		return;
	}
}

void SvnLogViewWidget::diffToPrevious()
{
    if( !m_ctxLogItem ){
        KMessageBox::error( this, i18n("No revision was clicked"), i18n("error") );
        return;
    }
    int revThis = m_ctxLogItem->text(0).toInt();
	int revPrev = revThis - 1;
	kdDebug(9036) << " Diff to prev requested on " << m_reqUrl << endl;
	m_part->svncore()->diffAsync( m_reqUrl, m_reqUrl, revPrev, "", revThis, "",
	 				true/*recurse*/, true/*peg_diff*/ );
}

SvnLogViewOptionDlg::SvnLogViewOptionDlg( QWidget *parent, const char* name, bool modal, WFlags f )
: SvnLogViewOptionDlgBase( parent, name, modal,f )
{
// 	radio1->setChecked(true); //repository log
	radio4->setChecked(true); //start revistion by revision keyword
	radio5->setChecked(true); //end revision by revision number
	reinstallRevisionSpecifiers();
	connect( intInput1, SIGNAL(valueChanged(int)), this, SLOT(setStartRevnumRadio()) );
	connect( comboBox1, SIGNAL(activated(const QString&)), this, SLOT(setStartRevkindRadio()) );
	connect( intInput2, SIGNAL(valueChanged(int)), this, SLOT(setEndRevnumRadio()) );
	connect( comboBox2, SIGNAL(activated(const QString&)), this, SLOT(setEndRevkindRadio()) );
}
SvnLogViewOptionDlg::~SvnLogViewOptionDlg()
{}
void SvnLogViewOptionDlg::reinstallRevisionSpecifiers()
{
	comboBox1->clear();
	comboBox2->clear();

	QStringList items;
	items << "HEAD" << "BASE" << "PREV" << "COMMITTED";
	comboBox1->insertStringList( items );
	comboBox2->insertStringList( items );
}
int SvnLogViewOptionDlg::revstart()
{
	if( !radio3->isChecked() ){
		return -1;
	} else{
		return intInput1->value();
	}
}
QString SvnLogViewOptionDlg::revKindStart()
{
	if( !radio4->isChecked() ){
		return QString("");
	} else{
		return comboBox1->currentText();
	}
}
int SvnLogViewOptionDlg::revend()
{
	if( !radio5->isChecked() ){
		return -1;
	} else{
		return intInput2->value();
	}
}
QString SvnLogViewOptionDlg::revKindEnd()
{
	if( !radio6->isChecked() ){
		return QString("");
	} else{
		return comboBox2->currentText();
	}
}
bool SvnLogViewOptionDlg::strictNode()
{
	if( checkBox1->isChecked() ){
		return true;
	} else{
		return false;
	}
}
void SvnLogViewOptionDlg::setStartRevnumRadio()
{
	radio3->setChecked(true);
}
void SvnLogViewOptionDlg::setStartRevkindRadio()
{
	radio4->setChecked(true);
}
void SvnLogViewOptionDlg::setEndRevnumRadio()
{
	radio5->setChecked(true);
}
void SvnLogViewOptionDlg::setEndRevkindRadio()
{
	radio6->setChecked(true);
}

#include "svn_logviewwidget.moc"

