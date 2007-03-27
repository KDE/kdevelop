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
#include "subversion_core.h"
#include <qtextedit.h>
#include <klocale.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <knuminput.h>
#include <qcheckbox.h>

SvnLogViewWidget::SvnLogViewWidget(subversionPart *part, QWidget *parent)
	:SvnLogViewWidgetBase(parent)
{
	m_part = part;
	connected = connect( listView1, SIGNAL(clicked( QListViewItem *)), this, SLOT(slotClicked(QListViewItem*)) );
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
	this->listView1->show();
}

void SvnLogViewWidget::append(QString txt)
{
	this->textEdit1->clear();// todo eventually, notification message and log history will be maintained separately.
	this->textEdit1->append(txt);
	this->listView1->hide();
}

void SvnLogViewWidget::slotClicked( QListViewItem *oneItem )
{
	if( !oneItem ) return;
	SvnLogViewItem *item = (SvnLogViewItem*)oneItem;
	textEdit1->clear();
	textEdit1->append( item->m_pathList );
	textEdit1->append( "\n\n" );
	textEdit1->append( item->m_message + "\n" );
}

SvnLogViewItem::SvnLogViewItem( QListView * parent )
	:SvnIntSortListItem( parent )
{
	m_pathList = "";
	m_message = "";
}
SvnLogViewItem ::~SvnLogViewItem ()
{}

SvnLogViewOptionDlg::SvnLogViewOptionDlg( QWidget *parent, const char* name, bool modal, WFlags f )
: SvnLogViewOptionDlgBase( parent, name, modal,f )
{
	radio1->setChecked(true); //repository log
	radio4->setChecked(true); //start revistion by revision keyword
	radio5->setChecked(true); //end revision by revision number
	reinstallRevisionSpecifiers( QButton::On );
	connect( radio1, SIGNAL(stateChanged(int)), this, SLOT(reinstallRevisionSpecifiers(int)) );
	connect( intInput1, SIGNAL(valueChanged(int)), this, SLOT(setStartRevnumRadio()) );
	connect( comboBox1, SIGNAL(activated(QString)), this, SLOT(setStartRevkindRadio()) );
	connect( intInput2, SIGNAL(valueChanged(int)), this, SLOT(setEndRevnumRadio()) );
	connect( comboBox2, SIGNAL(activated(QString)), this, SLOT(setEndRevkindRadio()) );
}
SvnLogViewOptionDlg::~SvnLogViewOptionDlg()
{}
void SvnLogViewOptionDlg::reinstallRevisionSpecifiers(int repositState )
{
	if( repositState == QButton::NoChange ){
		return;
	}
	comboBox1->clear();
	comboBox2->clear();
	if( repositState == QButton::On ){
		comboBox1->insertItem( "HEAD" );//there is no way to i18n subversion unique keywords
		comboBox2->insertItem( "HEAD" );//so don't try to translate these:-)
	} else if( repositState == QButton::Off ){
		QStringList items;
		items << "HEAD" << "BASE" << "PREV" << "COMMITTED";
		comboBox1->insertStringList( items );
		comboBox2->insertStringList( items );
	}
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
bool SvnLogViewOptionDlg::repositLog()
{
	if( radio1->isChecked() ){
		return true;
	} else{
		return false;
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

