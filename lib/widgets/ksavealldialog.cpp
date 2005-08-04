/* This file is part of the KDE project
   Copyright (C) 2002 Harald Fernengel <harry@kdevelop.org>

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

#include <q3vbox.h>
#include <qlabel.h>
#include <q3header.h>

#include <klocale.h>
#include <kpushbutton.h>
#include <klistbox.h>
#include <klistview.h>
#include <kstdguiitem.h>

#include "ksavealldialog.h"

namespace
{

class CheckURL : public Q3CheckListItem
{
public:
	CheckURL( Q3ListView * lv, KURL const & url )
		: Q3CheckListItem( lv, url.path(), Q3CheckListItem::CheckBox),
		_url( url )
	{}

	KURL const & url() const { return _url; }

private:
    KURL _url;
};

}


KSaveSelectDialog::KSaveSelectDialog( KURL::List const & filelist, KURL::List const & ignorelist, QWidget * parent ) :
  KDialogBase( parent, "SaveAllDialog", true, i18n("Save Modified Files?"),
	       Ok | User1 | Close )
{
  Q3VBox *top = makeVBoxMainWidget();

  (void)new QLabel( i18n("The following files have been modified. Save them?"), top );

	_listview = new KListView( top );
	_listview->addColumn( "" );
	_listview->header()->hide();
	_listview->setResizeMode( Q3ListView::LastColumn );

	setButtonOK( KGuiItem(i18n("Save &Selected"), QString::null, i18n("Saves all selected files")) );
	setButtonText( User1, i18n("Save &None") );
	setButtonText( Close, KStdGuiItem::cancel().text() );
	setButtonTip( User1, i18n("Lose all modifications") );
	setButtonTip( Close, i18n("Cancels the action") );
	
	KURL::List::ConstIterator it = filelist.begin();
	while ( it != filelist.end() )
	{
		if ( !ignorelist.contains( *it ) )
		{
			Q3CheckListItem * x = new CheckURL( _listview, *it );
			x->setOn( true );
		}
		++it;
	}

	connect( this, SIGNAL(closeClicked()), this, SLOT(cancel()) );
	connect( this, SIGNAL(okClicked()), this, SLOT(save()) );
	connect( this, SIGNAL(user1Clicked()), this, SLOT(saveNone()) );
}

KSaveSelectDialog::~KSaveSelectDialog() {}

void KSaveSelectDialog::saveNone( )
{
	// deselect all
	CheckURL * item = static_cast<CheckURL*>( _listview->firstChild() );
	while ( item )
	{
		item->setOn( false );
		item = static_cast<CheckURL*>( item->nextSibling() );
	}

	QDialog::accept();
}

void KSaveSelectDialog::save( )
{
	QDialog::accept();
}

void KSaveSelectDialog::cancel( )
{
	QDialog::reject();
}

KURL::List KSaveSelectDialog::filesToSave( )
{
	KURL::List filelist;
	CheckURL const * item = static_cast<CheckURL*>( _listview->firstChild() );
	while ( item )
	{
		if ( item->isOn() )
		{
			filelist << item->url();
		}
		item = static_cast<CheckURL*>( item->nextSibling() );
	}
	return filelist;
}

KURL::List KSaveSelectDialog::filesNotToSave( )
{
	KURL::List filelist;
	CheckURL const * item = static_cast<CheckURL*>( _listview->firstChild() );
	while ( item )
	{
		if ( ! item->isOn() )
		{
			filelist << item->url();
		}
		item = static_cast<CheckURL*>( item->nextSibling() );
	}
	return filelist;
}


KSaveAllDialog::KSaveAllDialog( const QStringList& filenames, QWidget* parent ) :
  KDialogBase( parent, "SaveAllDialog", true, i18n("Save Modified Files?"),
	       Ok | User1 | Close )
{
  m_result = Cancel;

  Q3VBox *top = makeVBoxMainWidget();

  (void)new QLabel( i18n("The following files have been modified. Save them?"), top );
  KListBox* lb = new KListBox( top );
  lb->setMinimumHeight( lb->fontMetrics().height() * 5 );
  lb->insertStringList( filenames );

  setButtonOK( KGuiItem(i18n("Save &All"), QString::null, i18n("Saves all modified files")) );
  setButtonText( User1, i18n("Save &None") );
  setButtonText( Close, KStdGuiItem::cancel().text() );
  setButtonTip( User1, i18n("Lose all modifications") );
  setButtonTip( Close, i18n("Cancels the action") );

  connect( this, SIGNAL(closeClicked()), this, SLOT(cancel()) );
  connect( this, SIGNAL(okClicked()), this, SLOT(saveAll()) );
  connect( this, SIGNAL(user1Clicked()), this, SLOT(revert()) );
}

KSaveAllDialog::~KSaveAllDialog()
{
}

void KSaveAllDialog::revert()
{
  m_result = Revert;
  QDialog::accept();
}

void KSaveAllDialog::saveAll()
{
  m_result = SaveAll;
  QDialog::accept();
}

void KSaveAllDialog::cancel()
{
  m_result = Cancel;
  QDialog::reject();
}

#include "ksavealldialog.moc"
