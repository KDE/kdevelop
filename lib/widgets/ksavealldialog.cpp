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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QLabel>
#include <QTreeWidget>
#include <QListWidget>
#include <QHeaderView>
#include <QWidget>

#include <klocale.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>

#include <kvbox.h>

#include <kdevdocument.h>

#include "ksavealldialog.h"

namespace
{

class CheckURL : public QTreeWidgetItem
{
public:
	CheckURL( QTreeWidget * lv, KDevDocument* document )
		: QTreeWidgetItem( lv),
		_document( document )
	{
            setText(0, document->url().path());
            setFlags(flags() | Qt::ItemIsUserCheckable);
        }

	KDevDocument* document() const { return _document; }

        bool isOn() const
        { return checkState(0) == Qt::Checked; }

        void setOn(bool b)
        { setCheckState(0, b ? Qt::Checked : Qt::Unchecked); }

private:
    KDevDocument* _document;
};

}


KSaveSelectDialog::KSaveSelectDialog( const QList<KDevDocument*>& filelist, const QList<KDevDocument*>& ignorelist, QWidget * parent ) :
  KDialog( parent )
{
  KVBox *top = new KVBox(this);
  setMainWidget(top);

  (void)new QLabel( i18n("The following files have been modified. Save them?"), top );

	_listview = new QTreeWidget( top );
	_listview->setColumnCount(1);
	_listview->header()->hide();
	_listview->header()->setResizeMode(0, QHeaderView::Stretch);

	setButtonGuiItem( Ok, KGuiItem(i18n("Save &Selected"), QString::null, i18n("Saves all selected files")) );
	setButtonText( User1, i18n("Save &None") );
	setButtonText( Close, KStdGuiItem::cancel().text() );
	
	foreach (KDevDocument* document, filelist )
	{
		if ( !ignorelist.contains( document ) )
		{
			CheckURL* x = new CheckURL( _listview, document );
			x->setOn( true );
		}
	}

	connect( this, SIGNAL(closeClicked()), this, SLOT(cancel()) );
	connect( this, SIGNAL(okClicked()), this, SLOT(save()) );
	connect( this, SIGNAL(user1Clicked()), this, SLOT(saveNone()) );
}

KSaveSelectDialog::~KSaveSelectDialog() {}

void KSaveSelectDialog::saveNone( )
{
	// deselect all
        for (int i=0; i<_listview->topLevelItemCount(); ++i) {
	   CheckURL * item = static_cast<CheckURL*>( _listview->topLevelItem(i) );
           item->setOn( false );
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

QList<KDevDocument*> KSaveSelectDialog::filesToSave( )
{
    QList<KDevDocument*> filelist;

    for (int i=0; i<_listview->topLevelItemCount(); ++i) {
        CheckURL * item = static_cast<CheckURL*>( _listview->topLevelItem(i) );

        if ( item->isOn() )
            filelist << item->document();
    }

    return filelist;
}

QList<KDevDocument*> KSaveSelectDialog::filesNotToSave( )
{
    QList<KDevDocument*> filelist;

    for (int i=0; i<_listview->topLevelItemCount(); ++i) {
        CheckURL * item = static_cast<CheckURL*>( _listview->topLevelItem(i) );

        if ( ! item->isOn() )
            filelist << item->document();
    }

    return filelist;
}


KSaveAllDialog::KSaveAllDialog( const QStringList& filenames, QWidget* parent ) :
  KDialog( parent )
{
  m_result = Cancel;

  KVBox *top = new KVBox(this);
  setMainWidget(top);

  (void)new QLabel( i18n("The following files have been modified. Save them?"), top );
  QListWidget* lb = new QListWidget( top );
  lb->setMinimumHeight( lb->fontMetrics().height() * 5 );
  lb->addItems( filenames );

  setButtonGuiItem( Ok, KGuiItem(i18n("Save &All"), QString::null, i18n("Saves all modified files")) );
  setButtonText( User1, i18n("Save &None") );
  setButtonText( Close, KStdGuiItem::cancel().text() );

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
